#include "Fighter.h"

#include "UniverseObjectVisitor.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"


Fighter::Fighter(int empire_id, int launched_from_id, const std::string& species_name,
                 float damage, const ::Condition::Condition* combat_targets/*, int current_turn*/) :
    UniverseObject(UniverseObjectType::OBJ_FIGHTER),
    m_damage(damage),
    m_launched_from_id(launched_from_id),
    m_species_name(species_name),
    m_combat_targets(combat_targets)
{
    this->SetOwner(empire_id);
    UniverseObject::Init();
}

bool Fighter::HostileToEmpire(int empire_id, const EmpireManager& empires) const {
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
        empires.GetDiplomaticStatus(Owner(), empire_id) == DiplomaticStatus::DIPLO_WAR;
}

const ::Condition::Condition* Fighter::CombatTargets() const
{ return m_combat_targets; }

float Fighter::Damage() const
{ return m_damage; }

bool Fighter::Destroyed() const
{ return m_destroyed; }

int Fighter::LaunchedFrom() const
{ return m_launched_from_id; }

const std::string& Fighter::SpeciesName() const
{ return m_species_name; }

void Fighter::SetDestroyed(bool destroyed)
{ m_destroyed = destroyed; }

std::string Fighter::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " (Combat Fighter) damage: " << m_damage;
    if (m_destroyed)
        os << "  (DESTROYED)";
    return os.str();
}

std::shared_ptr<UniverseObject> Fighter::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<Fighter>(std::static_pointer_cast<const Fighter>(shared_from_this()))); }

Fighter* Fighter::Clone(const Universe& universe, int empire_id) const {
    auto retval = std::make_unique<Fighter>();
    retval->Copy(shared_from_this(), universe, empire_id);
    return retval.release();
}

void Fighter::Copy(std::shared_ptr<const UniverseObject> copied_object,
                   const Universe& universe, int empire_id)
{
    if (copied_object.get() == this)
        return;
    auto copied_fighter = std::dynamic_pointer_cast<const Fighter>(copied_object);
    if (!copied_fighter) {
        ErrorLogger() << "Fighter::Copy passed an object that wasn't a Fighter";
        return;
    }

    UniverseObject::Copy(copied_object, Visibility::VIS_FULL_VISIBILITY, std::set<std::string>(), universe);

    this->m_damage = copied_fighter->m_damage;
    this->m_destroyed = copied_fighter->m_destroyed;
    this->m_combat_targets = copied_fighter->m_combat_targets;
}
