#include "ValueRefPythonParser.h"

#include <stdexcept>

#include <boost/python/extract.hpp>
#include <boost/python/str.hpp>
#include <boost/python/raw_function.hpp>

#include "../universe/ValueRefs.h"
#include "../universe/NamedValueRefManager.h"
#include "../universe/Conditions.h"

#include "EnumPythonParser.h"
#include "PythonParser.h"

value_ref_wrapper<double> pow(const value_ref_wrapper<double>& lhs, double rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::EXPONENTIATE,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator*(int lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
            std::make_unique<ValueRef::Constant<double>>(lhs),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator*(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
            std::make_unique<ValueRef::StaticCast<int, double>>(ValueRef::CloneUnique(lhs.value_ref)),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator*(const value_ref_wrapper<double>& lhs, double rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator*(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
            ValueRef::CloneUnique(lhs.value_ref),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator/(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::DIVIDE,
            ValueRef::CloneUnique(lhs.value_ref),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator/(const value_ref_wrapper<double>& lhs, int rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::DIVIDE,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator*(double lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
            std::make_unique<ValueRef::Constant<double>>(lhs),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator*(double lhs, const value_ref_wrapper<int>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::TIMES,
            std::make_unique<ValueRef::Constant<double>>(lhs),
            std::make_unique<ValueRef::StaticCast<int, double>>(ValueRef::CloneUnique(rhs.value_ref))
        )
    );
}

value_ref_wrapper<double> operator+(int lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            std::make_unique<ValueRef::Constant<double>>(lhs),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>& lhs, int rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>& lhs, double rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator+(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator-(const value_ref_wrapper<double>& lhs, double rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::MINUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

value_ref_wrapper<double> operator-(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::MINUS,
            ValueRef::CloneUnique(lhs.value_ref),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator-(int lhs, const value_ref_wrapper<double>& rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::MINUS,
            std::make_unique<ValueRef::Constant<double>>(lhs),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<double> operator-(const value_ref_wrapper<double>& lhs, int rhs) {
    return value_ref_wrapper<double>(
        std::make_shared<ValueRef::Operation<double>>(ValueRef::OpType::MINUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

condition_wrapper operator>=(const value_ref_wrapper<double>& lhs, int rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::GREATER_THAN_OR_EQUAL,
            std::make_unique<ValueRef::Constant<double>>(rhs)
        )
    );
}

condition_wrapper operator<=(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::LESS_THAN_OR_EQUAL,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

condition_wrapper operator<=(double lhs, const value_ref_wrapper<double>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(std::make_unique<ValueRef::Constant<double>>(lhs),
            Condition::ComparisonType::LESS_THAN_OR_EQUAL,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

condition_wrapper operator<=(const value_ref_wrapper<double>& lhs, double rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::LESS_THAN_OR_EQUAL,
            std::make_unique<ValueRef::Constant<double>>(rhs))
    );
}

condition_wrapper operator>(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::GREATER_THAN,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

condition_wrapper operator<(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::LESS_THAN,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

value_ref_wrapper<int> operator*(int lhs, const value_ref_wrapper<int>& rhs) {
    return value_ref_wrapper<int>(
        std::make_shared<ValueRef::Operation<int>>(ValueRef::OpType::TIMES,
            std::make_unique<ValueRef::Constant<int>>(lhs),
            ValueRef::CloneUnique(rhs.value_ref)
        )
    );
}

value_ref_wrapper<int> operator-(const value_ref_wrapper<int>& lhs, int rhs) {
    return value_ref_wrapper<int>(
        std::make_shared<ValueRef::Operation<int>>(ValueRef::OpType::MINUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<int>>(rhs)
        )
    );
}

value_ref_wrapper<int> operator+(const value_ref_wrapper<int>& lhs, int rhs) {
    return value_ref_wrapper<int>(
        std::make_shared<ValueRef::Operation<int>>(ValueRef::OpType::PLUS,
            ValueRef::CloneUnique(lhs.value_ref),
            std::make_unique<ValueRef::Constant<int>>(rhs)
        )
    );
}

condition_wrapper operator<(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::LESS_THAN,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

condition_wrapper operator==(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::EQUAL,
            ValueRef::CloneUnique(rhs.value_ref))
    );
}

condition_wrapper operator==(const value_ref_wrapper<int>& lhs, int rhs) {
    return condition_wrapper(
        std::make_shared<Condition::ValueTest>(ValueRef::CloneUnique(lhs.value_ref),
            Condition::ComparisonType::EQUAL,
            std::make_unique<ValueRef::Constant<int>>(rhs))
    );
}


namespace {
    template<typename T>
    value_ref_wrapper<T> insert_named_lookup_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();

        return value_ref_wrapper<T>(std::make_shared<ValueRef::NamedRef<T>>(name, /*is_lookup_only*/true));
    }

    template<typename T>
    value_ref_wrapper<T> insert_named_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();
        std::unique_ptr<ValueRef::ValueRef<T>> value;
        
        auto value_arg = boost::python::extract<value_ref_wrapper<T>>(kw["value"]);
        if (value_arg.check()) {
            value = ValueRef::CloneUnique(value_arg().value_ref);
        } else {
            value = std::make_unique<ValueRef::Constant<T>>(boost::python::extract<T>(kw["value"])());
        }

        ::RegisterValueRef<T>(name, std::move(value));

        return value_ref_wrapper<T>(std::make_shared<ValueRef::NamedRef<T>>(name));
    }


    boost::python::object insert_minmaxoneof_(const PythonParser& parser, ValueRef::OpType op, const boost::python::tuple& args, const boost::python::dict& kw) {
        if (args[0] == parser.type_int) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<int>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++) {
                auto arg = boost::python::extract<value_ref_wrapper<int>>(args[i]);
                if (arg.check())
                    operands.push_back(ValueRef::CloneUnique(arg().value_ref));
                else
                    operands.push_back(std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(args[i])()));
            }
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Operation<int>>(op, std::move(operands))));
        } else if (args[0] == parser.type_float) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<double>>> operands;
            operands.reserve(boost::python::len(args) - 1);
            for (auto i = 1; i < boost::python::len(args); i++) {
                auto arg = boost::python::extract<value_ref_wrapper<double>>(args[i]);
                if (arg.check())
                    operands.push_back(ValueRef::CloneUnique(arg().value_ref));
                else
                    operands.push_back(std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(args[i])()));
            }
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Operation<double>>(op, std::move(operands))));
        } else {
            ErrorLogger() << "Unsupported type for min/max/oneof : " << boost::python::extract<std::string>(boost::python::str(args[0]))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    boost::python::object insert_1arg_(const PythonParser& parser, const ValueRef::OpType op, const boost::python::tuple& args, const boost::python::dict& kw) {
        if (args[0] == parser.type_int) {
            std::unique_ptr<ValueRef::ValueRef<int>> operand;
            auto arg = boost::python::extract<value_ref_wrapper<int>>(args[1]);
            if (arg.check())
                operand = ValueRef::CloneUnique(arg().value_ref);
            else
                operand = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(args[1])());
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Operation<int>>(op, std::move(operand))));
        } else if (args[0] == parser.type_float) {
            std::unique_ptr<ValueRef::ValueRef<double>> operand;
            auto arg = boost::python::extract<value_ref_wrapper<double>>(args[1]);
            if (arg.check())
                operand = ValueRef::CloneUnique(arg().value_ref);
            else
                operand = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(args[1])());
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Operation<double>>(op, std::move(operand))));
        } else {
            ErrorLogger() << "Unsupported type for 1arg : " << boost::python::extract<std::string>(boost::python::str(args[0]))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    boost::python::object insert_statistic_(const PythonParser& parser, const ValueRef::StatisticType type, const boost::python::tuple& args, const boost::python::dict& kw) {
        auto condition = boost::python::extract<condition_wrapper>(kw["condition"])();
        if (args[0] == parser.type_int) {
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Statistic<int>>(nullptr, type, ValueRef::CloneUnique(condition.condition))));
        } else if (args[0] == parser.type_float) {
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Statistic<double>>(nullptr, type, ValueRef::CloneUnique(condition.condition))));
        } else {
            ErrorLogger() << "Unsupported type for statistic : " << boost::python::extract<std::string>(boost::python::str(args[0]))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    boost::python::object insert_statistic_value_(const PythonParser& parser, const boost::python::tuple& args, const boost::python::dict& kw) {
        const auto type = boost::python::extract<enum_wrapper<ValueRef::StatisticType>>(args[1])().value;
        const auto condition = boost::python::extract<condition_wrapper>(kw["condition"])().condition;

        if (args[0] == parser.type_int) {
            const auto value_arg = boost::python::extract<value_ref_wrapper<int>>(kw["value"]);
            std::unique_ptr<ValueRef::ValueRef<int>> value;
            if (value_arg.check()) {
                value = ValueRef::CloneUnique(value_arg().value_ref);
            } else {
                value = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["value"])());
            }
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::Statistic<int, int>>(std::move(value), type, ValueRef::CloneUnique(condition))));
        } else if (args[0] == parser.type_float) {
            const auto value_arg = boost::python::extract<value_ref_wrapper<double>>(kw["value"]);
            std::unique_ptr<ValueRef::ValueRef<double>> value;
            if (value_arg.check()) {
                value = ValueRef::CloneUnique(value_arg().value_ref);
            } else {
                value = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["value"])());
            }
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::Statistic<double, double>>(std::move(value), type, ValueRef::CloneUnique(condition))));
        } else {
            ErrorLogger() << "Unsupported type for statistic : " << boost::python::extract<std::string>(boost::python::str(args[0]))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    boost::python::object insert_game_rule_(const PythonParser& parser, const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto type_ = kw["type"];

        if (type_ == parser.type_int) {
            return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
                "GameRule",
                nullptr,
                nullptr,
                nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(name),
                nullptr)));
        } else if (type_ == parser.type_float) {
            return boost::python::object(value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
                "GameRule",
                nullptr,
                nullptr,
                nullptr,
                std::make_unique<ValueRef::Constant<std::string>>(name),
                nullptr)));
        } else {
            ErrorLogger() << "Unsupported type for rule " << name << ": " << boost::python::extract<std::string>(boost::python::str(type_))();

            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return boost::python::object();
    }

    boost::python::object insert_int_complex_variable_(const char* variable, const boost::python::tuple& args, const boost::python::dict& kw) {
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
        if (kw.has_key("name")) {
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
        }

        return boost::python::object(value_ref_wrapper<int>(std::make_shared<ValueRef::ComplexVariable<int>>(
            variable,
            std::move(empire),
            nullptr,
            nullptr,
            std::move(name),
            nullptr)));
    }

    value_ref_wrapper<double> insert_direct_distance_between_(boost::python::object arg1, boost::python::object arg2) {
        std::unique_ptr<ValueRef::ValueRef<int>> id1;
        auto id1_args = boost::python::extract<value_ref_wrapper<int>>(arg1);
        if (id1_args.check()) {
            id1 = ValueRef::CloneUnique(id1_args().value_ref);
        } else {
            id1 = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(arg1)());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> id2;
        auto id2_args = boost::python::extract<value_ref_wrapper<int>>(arg2);
        if (id2_args.check()) {
            id2 = ValueRef::CloneUnique(id2_args().value_ref);
        } else {
            id2 = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(arg2)());
        }

        return value_ref_wrapper<double>(std::make_shared<ValueRef::ComplexVariable<double>>(
            "DirectDistanceBetween",
            std::move(id1),
            std::move(id2),
            nullptr,
            nullptr,
            nullptr
        ));
    }
}

void RegisterGlobalsValueRefs(boost::python::dict& globals, const PythonParser& parser) {
    globals["NamedReal"] = boost::python::raw_function(insert_named_<double>);
    globals["NamedRealLookup"] = boost::python::raw_function(insert_named_lookup_<double>);
    globals["Value"] = value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(ValueRef::ReferenceType::EFFECT_TARGET_VALUE_REFERENCE));

    // free variable name
    for (const char* variable : {"CombatBout",
                                 "CurrentTurn",
                                 "GalaxyAge",
                                 "GalaxyMaxAIAggression",
                                 "GalaxyMonsterFrequency",
                                 "GalaxyNativeFrequency",
                                 "GalaxyPlanetDensity",
                                 "GalaxyShape",
                                 "GalaxySize",
                                 "GalaxySpecialFrequency",
                                 "GalaxyStarlaneFrequency",
                                 "UsedInDesignID"})
    {
        globals[variable] = value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(ValueRef::ReferenceType::NON_OBJECT_REFERENCE, variable));       
    }

    // Integer complex variables
    for (const char* variable : {"BuildingTypesOwned",
                                 "BuildingTypesProduced",
                                 "BuildingTypesScrapped",
                                 "SpeciesColoniesOwned",
                                 "SpeciesPlanetsBombed",
                                 "SpeciesPlanetsDepoped",
                                 "SpeciesPlanetsInvaded",
                                 "SpeciesShipsDestroyed",
                                 "SpeciesShipsLost",
                                 "SpeciesShipsOwned",
                                 "SpeciesShipsProduced",
                                 "SpeciesShipsScrapped",
                                 "TurnTechResearched",
                                 "TurnPolicyAdopted",
                                 "TurnsSincePolicyAdopted",
                                 "CumulativeTurnsPolicyAdopted",
                                 "NumPoliciesAdopted"})
    {
        std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_int_complex_variable = [variable](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_int_complex_variable_(variable, args, kw); };
        globals[variable] = boost::python::raw_function(f_insert_int_complex_variable);
    }

    // single-parameter math functions
    for (const auto& op : std::initializer_list<std::pair<const char*, ValueRef::OpType>>{
            {"Sin",   ValueRef::OpType::SINE},
            {"Cos",   ValueRef::OpType::COSINE},
            {"Log",   ValueRef::OpType::LOGARITHM},
            {"NoOp",  ValueRef::OpType::NOOP},
            {"Abs",   ValueRef::OpType::ABS},
            {"Round", ValueRef::OpType::ROUND_NEAREST},
            {"Ceil",  ValueRef::OpType::ROUND_UP},
            {"Floor", ValueRef::OpType::ROUND_DOWN},
            {"Sign",  ValueRef::OpType::SIGN}})
    {
        std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_abs = [&parser, op](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_1arg_(parser, op.second, args, kw); };
        globals[op.first] = boost::python::raw_function(f_insert_abs, 2);
    }

    std::function<boost::python::object(const boost::python::tuple&, const boost::python::dict&)> f_insert_game_rule = [&parser](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_game_rule_(parser, args, kw); };
    globals["GameRule"] = boost::python::raw_function(f_insert_game_rule);

    // selection_operator
    for (const auto& op : std::initializer_list<std::pair<const char*, ValueRef::OpType>>{
            {"OneOf",   ValueRef::OpType::RANDOM_PICK},
            {"MinOf",   ValueRef::OpType::MINIMUM},
            {"MaxOf",   ValueRef::OpType::MAXIMUM}})
    {
        const auto e = op.second;
        const auto f = [&parser, e](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_minmaxoneof_(parser, e, args, kw); };
        globals[op.first] = boost::python::raw_function(f, 3);
    }

    const auto f_insert_statistic_if = [&parser](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_statistic_(parser, ValueRef::StatisticType::IF, args, kw); };
    globals["StatisticIf"] = boost::python::raw_function(f_insert_statistic_if, 1);

    const auto f_insert_statistic_count = [&parser](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_statistic_(parser, ValueRef::StatisticType::COUNT, args, kw); };
    globals["StatisticCount"] = boost::python::raw_function(f_insert_statistic_count, 1);

    const auto f_insert_statistic = [&parser](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_statistic_value_(parser, args, kw); };
    globals["Statistic"] = boost::python::raw_function(f_insert_statistic, 2);

    globals["DirectDistanceBetween"] = insert_direct_distance_between_;
}

