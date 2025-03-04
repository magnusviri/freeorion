#include "ConditionPythonParser.h"

#include <boost/python/extract.hpp>
#include <boost/python/raw_function.hpp>

#include "../universe/Conditions.h"

#include "EnumPythonParser.h"
#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"

condition_wrapper operator&(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    std::shared_ptr<Condition::ValueTest> lhs_cond = std::dynamic_pointer_cast<Condition::ValueTest>(lhs.condition);
    std::shared_ptr<Condition::ValueTest> rhs_cond = std::dynamic_pointer_cast<Condition::ValueTest>(rhs.condition);

    if (lhs_cond && rhs_cond) {
        const auto lhs_vals = lhs_cond->ValuesDouble();
        const auto rhs_vals = rhs_cond->ValuesDouble();

        if (!lhs_vals[2] && !rhs_vals[2] && lhs_vals[1] && rhs_vals[0] && (*lhs_vals[1] == *rhs_vals[0])) {
            return condition_wrapper(std::make_shared<Condition::ValueTest>(
                lhs_vals[0] ? lhs_vals[0]->Clone() : nullptr,
                lhs_cond->CompareTypes()[0],
                lhs_vals[1]->Clone(),
                rhs_cond->CompareTypes()[0],
                rhs_vals[1] ? rhs_vals[1]->Clone() : nullptr
            ));
        }

        const auto lhs_vals_i = lhs_cond->ValuesInt();
        const auto rhs_vals_i = rhs_cond->ValuesInt();

        if (!lhs_vals_i[2] && !rhs_vals_i[2] && lhs_vals_i[1] && rhs_vals_i[0] && (*lhs_vals_i[1] == *rhs_vals_i[0])) {
            return condition_wrapper(std::make_shared<Condition::ValueTest>(
                lhs_vals_i[0] ? lhs_vals_i[0]->Clone() : nullptr,
                lhs_cond->CompareTypes()[0],
                lhs_vals_i[1]->Clone(),
                rhs_cond->CompareTypes()[0],
                rhs_vals_i[1] ? rhs_vals_i[1]->Clone() : nullptr
            ));
        }

        const auto lhs_vals_s = lhs_cond->ValuesString();
        const auto rhs_vals_s = rhs_cond->ValuesString();

        if (!lhs_vals_s[2] && !rhs_vals_s[2] && lhs_vals_s[1] && rhs_vals_s[0] && (*lhs_vals_s[1] == *rhs_vals_s[0])) {
            return condition_wrapper(std::make_shared<Condition::ValueTest>(
                lhs_vals_s[0] ? lhs_vals_s[0]->Clone() : nullptr,
                lhs_cond->CompareTypes()[0],
                lhs_vals_s[1]->Clone(),
                rhs_cond->CompareTypes()[0],
                rhs_vals_s[1] ? rhs_vals_s[1]->Clone() : nullptr
            ));
        }
    }

    return condition_wrapper(std::make_shared<Condition::And>(
        lhs.condition->Clone(),
        rhs.condition->Clone()
    ));
}

condition_wrapper operator|(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    return condition_wrapper(std::make_shared<Condition::Or>(
        lhs.condition->Clone(),
        rhs.condition->Clone()
    ));
}

condition_wrapper operator~(const condition_wrapper& lhs) {
    return condition_wrapper(std::make_shared<Condition::Not>(
        lhs.condition->Clone()
    ));
}


namespace {
    condition_wrapper insert_owned_by_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        EmpireAffiliationType affiliation = EmpireAffiliationType::AFFIL_SELF;

        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        if (kw.has_key("affiliation")) {
            affiliation = boost::python::extract<enum_wrapper<EmpireAffiliationType>>(kw["affiliation"])().value;
        }

        return condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(std::move(empire), affiliation));
    }

    condition_wrapper insert_contained_by_(const condition_wrapper& cond) {
        return condition_wrapper(std::make_shared<Condition::ContainedBy>(ValueRef::CloneUnique(cond.condition)));
    }

    condition_wrapper insert_contains_(const condition_wrapper& cond) {
        return condition_wrapper(std::make_shared<Condition::Contains>(ValueRef::CloneUnique(cond.condition)));
    }

    condition_wrapper insert_meter_value_(const boost::python::tuple& args, const boost::python::dict& kw, MeterType m) {
        std::unique_ptr<ValueRef::ValueRef<double>> low;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<double>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<double>> high;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<double>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["high"])());
            }
        }
        return condition_wrapper(std::make_shared<Condition::MeterValue>(m, std::move(low), std::move(high)));
    }

    condition_wrapper insert_visible_to_empire_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        if (kw.has_key("turn")) {
            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        if (kw.has_key("visibility")) {
            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return condition_wrapper(std::make_shared<Condition::VisibleToEmpire>(std::move(empire)));
    }

    condition_wrapper insert_planet_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("type")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>> types;
            py_parse::detail::flatten_list<boost::python::object>(kw["type"], [](const boost::python::object& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>>& v) {
                auto type_arg = boost::python::extract<value_ref_wrapper< ::PlanetType>>(o);
                if (type_arg.check()) {
                    v.push_back(ValueRef::CloneUnique(type_arg().value_ref));
                } else {
                    v.push_back(std::make_unique<ValueRef::Constant< ::PlanetType>>(boost::python::extract<enum_wrapper< ::PlanetType>>(o)().value));
                }
            }, types);
            return condition_wrapper(std::make_shared<Condition::PlanetType>(std::move(types)));
        } else if (kw.has_key("size")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>> sizes;
            py_parse::detail::flatten_list<boost::python::object>(kw["size"], [](const boost::python::object& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>>& v) {
                auto size_arg = boost::python::extract<value_ref_wrapper< ::PlanetSize>>(o);
                if (size_arg.check()) {
                    v.push_back(ValueRef::CloneUnique(size_arg().value_ref));
                } else {
                    v.push_back(std::make_unique<ValueRef::Constant< ::PlanetSize>>(boost::python::extract<enum_wrapper< ::PlanetSize>>(o)().value));
                }
            }, sizes);
            return condition_wrapper(std::make_shared<Condition::PlanetSize>(std::move(sizes)));
        } else if (kw.has_key("environment")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>> environments;
            py_parse::detail::flatten_list<boost::python::object>(kw["environment"], [](const boost::python::object& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>>& v) {
                auto env_arg = boost::python::extract<value_ref_wrapper< ::PlanetEnvironment>>(o);
                if (env_arg.check()) {
                    v.push_back(ValueRef::CloneUnique(env_arg().value_ref));
                } else {
                    v.push_back(std::make_unique<ValueRef::Constant< ::PlanetEnvironment>>(boost::python::extract<enum_wrapper< ::PlanetEnvironment>>(o)().value));
                }
            }, environments);
            return condition_wrapper(std::make_shared<Condition::PlanetEnvironment>(std::move(environments)));
        }
        return condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_PLANET));
    }

    condition_wrapper insert_has_tag_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        if (kw.has_key("name")) {
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
        }
        return condition_wrapper(std::make_shared<Condition::HasTag>(std::move(name)));
    }

    condition_wrapper insert_focus_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> types;
        py_parse::detail::flatten_list<boost::python::object>(kw["type"], [](const boost::python::object& o, std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>>& v) {
            auto type_arg = boost::python::extract<value_ref_wrapper<std::string>>(o);
            if (type_arg.check()) {
                v.push_back(ValueRef::CloneUnique(type_arg().value_ref));
            } else {
                v.push_back(std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(o)()));
            }
        }, types);
        return condition_wrapper(std::make_shared<Condition::FocusType>(std::move(types)));
    }

    condition_wrapper insert_empire_stockpile_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        auto resource = boost::python::extract<enum_wrapper<ResourceType>>(kw["resource"])();

        std::unique_ptr<ValueRef::ValueRef<double>> low;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<double>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<double>> high;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<double>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["high"])());
            }
        }

        return condition_wrapper(std::make_shared<Condition::EmpireStockpileValue>(
            std::move(empire),
            resource.value,
            std::move(low),
            std::move(high)));
    }

    condition_wrapper insert_empire_has_adopted_policy_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }

        return condition_wrapper(std::make_shared<Condition::EmpireHasAdoptedPolicy>(
            std::move(empire),
            std::move(name)));
    }

    condition_wrapper insert_resupplyable_by_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        return condition_wrapper(std::make_shared<Condition::FleetSupplyableByEmpire>(std::move(empire)));
    }

    condition_wrapper insert_owner_has_tech_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }
        return condition_wrapper(std::make_shared<Condition::OwnerHasTech>(std::move(name)));
    }

    condition_wrapper insert_random_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<double>> probability;
        auto p_args = boost::python::extract<value_ref_wrapper<double>>(kw["probability"]);
        if (p_args.check()) {
            probability = ValueRef::CloneUnique(p_args().value_ref);
        } else {
            probability = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["probability"])());
        }
        return condition_wrapper(std::make_shared<Condition::Chance>(std::move(probability)));
    }

    condition_wrapper insert_star_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::vector<std::unique_ptr<ValueRef::ValueRef< ::StarType>>> types;

        py_parse::detail::flatten_list<boost::python::object>(kw["type"], [](const boost::python::object& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::StarType>>>& v) {
            auto type_arg = boost::python::extract<value_ref_wrapper< ::StarType>>(o);
            if (type_arg.check()) {
                v.push_back(ValueRef::CloneUnique(type_arg().value_ref));
            } else {
                v.push_back(std::make_unique<ValueRef::Constant< ::StarType>>(boost::python::extract<enum_wrapper< ::StarType>>(o)().value));
            }
        }, types);

        return condition_wrapper(std::make_shared<Condition::StarType>(std::move(types)));
    }

    condition_wrapper insert_in_system_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> system_id;
        if (kw.has_key("id")) {
            auto id_args = boost::python::extract<value_ref_wrapper<int>>(kw["id"]);
            if (id_args.check()) {
                system_id = ValueRef::CloneUnique(id_args().value_ref);
            } else {
                system_id = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["id"])());
            }
        }
        
        return condition_wrapper(std::make_shared<Condition::InOrIsSystem>(std::move(system_id)));
    }

    condition_wrapper insert_turn_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<int>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<int>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["high"])());
            }
        }

        return condition_wrapper(std::make_shared<Condition::Turn>(std::move(low), std::move(high)));
    }
}

void RegisterGlobalsConditions(boost::python::dict& globals) {
    globals["Ship"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_SHIP));
    globals["System"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_SYSTEM));
    globals["Fleet"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_FLEET));
    globals["ProductionCenter"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_PROD_CENTER));
    globals["Monster"] = condition_wrapper(std::make_shared<Condition::Monster>());
    globals["Capital"] = condition_wrapper(std::make_shared<Condition::Capital>());
    globals["Stationary"] = condition_wrapper(std::make_shared<Condition::Stationary>());

    globals["Unowned"] = condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_NONE));
    globals["Human"] = condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_HUMAN));

    globals["OwnerHasTech"] = boost::python::raw_function(insert_owner_has_tech_);
    globals["Random"] = boost::python::raw_function(insert_random_);
    globals["Star"] = boost::python::raw_function(insert_star_);
    globals["InSystem"] = boost::python::raw_function(insert_in_system_);
    globals["ResupplyableBy"] = boost::python::raw_function(insert_resupplyable_by_);

    // non_ship_part_meter_enum_grammar
    for (const auto& meter : std::initializer_list<std::pair<const char*, MeterType>>{
            {"TargetConstruction", MeterType::METER_TARGET_CONSTRUCTION},
            {"TargetIndustry",     MeterType::METER_TARGET_INDUSTRY},
            {"TargetPopulation",   MeterType::METER_TARGET_POPULATION},
            {"TargetResearch",     MeterType::METER_TARGET_RESEARCH},
            {"TargetInfluence",    MeterType::METER_TARGET_INFLUENCE},
            {"TargetHappiness",    MeterType::METER_TARGET_HAPPINESS},
            {"MaxDefense",         MeterType::METER_MAX_DEFENSE},
            {"MaxFuel",            MeterType::METER_MAX_FUEL},
            {"MaxShield",          MeterType::METER_MAX_SHIELD},
            {"MaxStructure",       MeterType::METER_MAX_STRUCTURE},
            {"MaxTroops",          MeterType::METER_MAX_TROOPS},
            {"MaxSupply",          MeterType::METER_MAX_SUPPLY},
            {"MaxStockpile",       MeterType::METER_MAX_STOCKPILE},

            {"Construction",       MeterType::METER_CONSTRUCTION},
            {"Industry",           MeterType::METER_INDUSTRY},
            {"Population",         MeterType::METER_POPULATION},
            {"Research",           MeterType::METER_RESEARCH},
            {"Influence",          MeterType::METER_INFLUENCE},
            {"Happiness",          MeterType::METER_HAPPINESS},

            {"Defense",            MeterType::METER_DEFENSE},
            {"Fuel",               MeterType::METER_FUEL},
            {"Shield",             MeterType::METER_SHIELD},
            {"Structure",          MeterType::METER_STRUCTURE},
            {"Troops",             MeterType::METER_TROOPS},
            {"Supply",             MeterType::METER_SUPPLY},
            {"Stockpile",          MeterType::METER_STOCKPILE},

            {"RebelTroops",        MeterType::METER_REBEL_TROOPS},
            {"Stealth",            MeterType::METER_STEALTH},
            {"Detection",          MeterType::METER_DETECTION},
            {"Speed",              MeterType::METER_SPEED},

            {"Size",               MeterType::METER_SIZE}})
    {
        const auto m = meter.second;
        const auto f_insert_meter_value = [m](const auto& args, const auto& kw) { return insert_meter_value_(args, kw, m); };
        globals[meter.first] = boost::python::raw_function(f_insert_meter_value);
    }

    globals["Species"] = condition_wrapper(std::make_shared<Condition::Species>());
    globals["CanColonize"] = condition_wrapper(std::make_shared<Condition::CanColonize>());

    globals["HasTag"] = boost::python::raw_function(insert_has_tag_);
    globals["Planet"] = boost::python::raw_function(insert_planet_);
    globals["VisibleToEmpire"] = boost::python::raw_function(insert_visible_to_empire_);
    globals["OwnedBy"] = boost::python::raw_function(insert_owned_by_);
    globals["ContainedBy"] = insert_contained_by_;
    globals["Contains"] = insert_contains_;
    globals["Focus"] = boost::python::raw_function(insert_focus_);
    globals["EmpireStockpile"] = boost::python::raw_function(insert_empire_stockpile_);
    globals["EmpireHasAdoptedPolicy"] = boost::python::raw_function(insert_empire_has_adopted_policy_);
    globals["Turn"] = boost::python::raw_function(insert_turn_);
}

