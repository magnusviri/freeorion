from common.base_prod import TECH_COST_MULTIPLIER
from common.priorities import TARGET_POPULATION_BEFORE_SCALING_PRIORITY

Tech(
    name="GRO_SYMBIOTIC_BIO",
    description="GRO_SYMBIOTIC_BIO_DESC",
    short_description="POPULATION_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=50 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites="GRO_PLANET_ECOL",
    unlock=[
        Item(type=UnlockPolicy, name="PLC_DIVERSITY"),
        Item(type=UnlockPolicy, name="PLC_BLACK_MARKET"),
    ],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & Planet(
                environment=[
                    Good,
                    Adequate,
                    Poor,
                ]
            ),
            accountinglabel="GRO_TECH_ACCOUNTING_LABEL",
            priority=TARGET_POPULATION_BEFORE_SCALING_PRIORITY,
            effects=SetTargetPopulation(value=Value + 1 * Target.HabitableSize),
        ),
    ],
    graphic="icons/tech/symbiosis_biology.png",
)
