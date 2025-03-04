#include "Building.h"

#include "BuildingType.h"
#include "UniverseObjectVisitor.h"
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"
#include "../util/i18n.h"


Building::Building(int empire_id, std::string building_type, int produced_by_empire_id,
                   int creation_turn) :
    UniverseObject{UniverseObjectType::OBJ_BUILDING, "", empire_id, creation_turn},
    m_building_type(std::move(building_type)),
    m_produced_by_empire_id(produced_by_empire_id)
{
    const BuildingType* type = GetBuildingType(m_building_type);
    Rename(type ? UserString(type->Name()) : UserString("ENC_BUILDING"));

    UniverseObject::Init();
}

Building* Building::Clone(const Universe& universe, int empire_id) const {
    Visibility vis = universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    auto retval = std::make_unique<Building>();
    retval->Copy(shared_from_this(), universe, empire_id);
    return retval.release();
}

void Building::Copy(std::shared_ptr<const UniverseObject> copied_object,
                    const Universe& universe, int empire_id)
{
    if (copied_object.get() == this)
        return;
    auto copied_building = std::dynamic_pointer_cast<const Building>(copied_object);
    if (!copied_building) {
        ErrorLogger() << "Building::Copy passed an object that wasn't a Building";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = universe.GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    auto visible_specials = universe.GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(std::move(copied_object), vis, visible_specials, universe);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        this->m_planet_id =                 copied_building->m_planet_id;

        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            this->m_name =                  copied_building->m_name;

            this->m_building_type =         copied_building->m_building_type;
            this->m_produced_by_empire_id = copied_building->m_produced_by_empire_id;

            if (vis >= Visibility::VIS_FULL_VISIBILITY)
                this->m_ordered_scrapped =  copied_building->m_ordered_scrapped;
        }
    }
}

bool Building::HostileToEmpire(int empire_id, const EmpireManager& empires) const {
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
        empires.GetDiplomaticStatus(Owner(), empire_id) == DiplomaticStatus::DIPLO_WAR;
}

UniverseObject::TagVecs Building::Tags(const ScriptingContext&) const {
    if (const BuildingType* type = ::GetBuildingType(m_building_type))
        return type->Tags();
    return {};
}

bool Building::HasTag(std::string_view name, const ScriptingContext&) const {
    const BuildingType* type = GetBuildingType(m_building_type);
    return type && type->HasTag(name);
}

bool Building::ContainedBy(int object_id) const {
    return object_id != INVALID_OBJECT_ID
        && (    object_id == m_planet_id
            ||  object_id == this->SystemID());
}

std::string Building::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " building type: " << m_building_type
       << " produced by empire id: " << m_produced_by_empire_id;
    return os.str();
}

std::shared_ptr<UniverseObject> Building::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<Building>(std::static_pointer_cast<const Building>(shared_from_this()))); }

void Building::SetPlanetID(int planet_id) {
    if (planet_id != m_planet_id) {
        m_planet_id = planet_id;
        StateChangedSignal();
    }
}

void Building::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    //// give buildings base stealth slightly above 0, so that they can't be seen from a distance without high detection ability
    //if (Meter* stealth = GetMeter(MeterType::METER_STEALTH))
    //    stealth->AddToCurrent(0.01f);
}

void Building::Reset() {
    UniverseObject::SetOwner(ALL_EMPIRES);
    m_ordered_scrapped = false;
}

void Building::SetOrderedScrapped(bool b) {
    bool initial_status = m_ordered_scrapped;
    if (b == initial_status) return;
    m_ordered_scrapped = b;
    StateChangedSignal();
}

