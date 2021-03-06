#include "Events/ItemEvents.hpp"
#include "API/CAppManager.hpp"
#include "API/CServerExoApp.hpp"
#include "API/CNWSCreature.hpp"
#include "API/Functions.hpp"
#include "API/CNWSItem.hpp"
#include "API/CItemRepository.hpp"
#include "API/Constants.hpp"
#include "API/Globals.hpp"
#include "Events.hpp"
#include "Utils.hpp"

namespace Events {

using namespace NWNXLib;
using namespace NWNXLib::API;
using namespace NWNXLib::API::Constants;

static Hooking::FunctionHook* m_UseItemHook = nullptr;
static Hooking::FunctionHook* m_OpenInventoryHook = nullptr;
static Hooking::FunctionHook* m_CloseInventoryHook = nullptr;
static Hooking::FunctionHook* m_AddItemHook = nullptr;

ItemEvents::ItemEvents(ViewPtr<Services::HooksProxy> hooker)
{
    hooker->RequestExclusiveHook<API::Functions::CNWSCreature__UseItem>(&UseItemHook);
    m_UseItemHook = hooker->FindHookByAddress(API::Functions::CNWSCreature__UseItem);

    hooker->RequestExclusiveHook<API::Functions::CNWSItem__OpenInventory>(&OpenInventoryHook);
    m_OpenInventoryHook = hooker->FindHookByAddress(API::Functions::CNWSItem__OpenInventory);

    hooker->RequestExclusiveHook<API::Functions::CNWSItem__CloseInventory>(&CloseInventoryHook);
    m_CloseInventoryHook = hooker->FindHookByAddress(API::Functions::CNWSItem__CloseInventory);

    hooker->RequestExclusiveHook<API::Functions::CItemRepository__AddItem>(&AddItemHook);
    m_AddItemHook = hooker->FindHookByAddress(API::Functions::CItemRepository__AddItem);

    hooker->RequestSharedHook<API::Functions::CItemRepository__RemoveItem, int32_t>(&RemoveItemHook);
}

int32_t ItemEvents::UseItemHook(
    API::CNWSCreature* thisPtr,
    API::Types::ObjectID item,
    uint8_t propIndex,
    uint8_t subPropIndex,
    API::Types::ObjectID target,
    API::Vector targetPosition,
    API::Types::ObjectID area)
{
    int32_t retVal;

    auto PushAndSignal = [&](std::string ev) -> bool {
        Events::PushEventData("ITEM_OBJECT_ID", Utils::ObjectIDToString(item));
        Events::PushEventData("TARGET_OBJECT_ID", Utils::ObjectIDToString(target));
        Events::PushEventData("ITEM_PROPERTY_INDEX", std::to_string(propIndex));
        Events::PushEventData("ITEM_SUB_PROPERTY_INDEX", std::to_string(subPropIndex));
        Events::PushEventData("TARGET_POSITION_X", std::to_string(targetPosition.x));
        Events::PushEventData("TARGET_POSITION_Y", std::to_string(targetPosition.y));
        Events::PushEventData("TARGET_POSITION_Z", std::to_string(targetPosition.z));
    return Events::SignalEvent(ev, thisPtr->m_idSelf);
    };

    if (PushAndSignal("NWNX_ON_USE_ITEM_BEFORE"))
    {
        retVal = m_UseItemHook->CallOriginal<int32_t>(thisPtr, item, propIndex, subPropIndex, target, targetPosition, area);
    }
    else
    {
        retVal = false;
    }

    PushAndSignal("NWNX_ON_USE_ITEM_AFTER");

    return retVal;
}

void ItemEvents::OpenInventoryHook(CNWSItem* thisPtr, Types::ObjectID oidOpener)
{
    auto PushAndSignal = [&](std::string ev) -> bool {
        Events::PushEventData("OWNER", Utils::ObjectIDToString(oidOpener));
        return Events::SignalEvent(ev, thisPtr->m_idSelf);
    };

    if (PushAndSignal("NWNX_ON_ITEM_INVENTORY_OPEN_BEFORE"))
    {
        m_OpenInventoryHook->CallOriginal<void>(thisPtr, oidOpener);
    }

    PushAndSignal("NWNX_ON_ITEM_INVENTORY_OPEN_AFTER");
}

void ItemEvents::CloseInventoryHook(CNWSItem* thisPtr, Types::ObjectID oidCloser, int32_t bUpdatePlayer)
{
    auto PushAndSignal = [&](std::string ev) -> bool {
        Events::PushEventData("OWNER", Utils::ObjectIDToString(oidCloser));
        return Events::SignalEvent(ev, thisPtr->m_idSelf);
    };

    if (PushAndSignal("NWNX_ON_ITEM_INVENTORY_CLOSE_BEFORE"))
    {
        m_CloseInventoryHook->CallOriginal<void>(thisPtr, oidCloser, bUpdatePlayer);
    }

    PushAndSignal("NWNX_ON_ITEM_INVENTORY_CLOSE_AFTER");
}

int32_t ItemEvents::AddItemHook(CItemRepository* thisPtr, CNWSItem** ppItem, uint8_t x, uint8_t y,
                                int32_t bAllowEncumbrance, int32_t bMergeItem)
{
    int32_t retVal;
    auto *pItemContainer = Utils::AsNWSItem(Globals::AppManager()->m_pServerExoApp->GetGameObject(thisPtr->m_oidParent));

    if(!pItemContainer)
    {// Early out if parent isn't an item or Bad Things(tm) happen
        return m_AddItemHook->CallOriginal<int32_t>(thisPtr, ppItem, x, y, bAllowEncumbrance, bMergeItem);
    }

    auto PushAndSignal = [&](std::string ev) -> bool {
        Events::PushEventData("ITEM", Utils::ObjectIDToString(ppItem && *ppItem ? (**ppItem).m_idSelf : OBJECT_INVALID));
        return Events::SignalEvent(ev, thisPtr->m_oidParent);
    };

    if (PushAndSignal("NWNX_ON_ITEM_INVENTORY_ADD_ITEM_BEFORE"))
    {
        retVal = m_AddItemHook->CallOriginal<int32_t>(thisPtr, ppItem, x, y, bAllowEncumbrance, bMergeItem);
    }
    else
    {
        retVal = false;
    }

    PushAndSignal("NWNX_ON_ITEM_INVENTORY_ADD_ITEM_AFTER");

    return retVal;
}

void ItemEvents::RemoveItemHook(Services::Hooks::CallType type, CItemRepository* thisPtr, CNWSItem* pItem)
{
    auto *pItemContainer = Utils::AsNWSItem(Globals::AppManager()->m_pServerExoApp->GetGameObject(thisPtr->m_oidParent));

    if(!pItemContainer)
    {// Early out if parent isn't an item or Bad Things(tm) happen
        return;
    }

    // Only a shared hook for RemoveItem because skipping it also makes Bad Things(tm) happen
    const bool before = type == Services::Hooks::CallType::BEFORE_ORIGINAL;
    Events::PushEventData("ITEM", Utils::ObjectIDToString(pItem ? pItem->m_idSelf : OBJECT_INVALID));
    Events::SignalEvent(before ? "NWNX_ON_ITEM_INVENTORY_REMOVE_ITEM_BEFORE" : "NWNX_ON_ITEM_INVENTORY_REMOVE_ITEM_AFTER", thisPtr->m_oidParent);
}

}
