struct TFgPilotReadGroup {
    uint16 startSector;
    uint16 endSector;
    uint16 minSlackVBlanks;
};

struct TFgRuntimeWindowPolicy {
    const char *sceneName;
    uint32 highWindowBytes;
    uint32 lowWindowBytes;
    uint8 requiredPackFormat;
};

struct TFgRuntimeSetupPrimePolicy {
    const char *sceneName;
    uint32 highWindowBytes;
    uint32 lowWindowBytes;
};

static const struct TFgRuntimeWindowPolicy kFgRuntimeWindowPolicies[] = {
    {"activity9", FG_ACTIVITY9_HIGH_WINDOW_BYTES, FG_ACTIVITY9_LOW_WINDOW_BYTES, 0},
    {"building4", FG_BUILDING4_HIGH_WINDOW_BYTES, FG_BUILDING4_LOW_WINDOW_BYTES, 0},
    {"building6", FG_BUILDING6_WINDOW_BYTES, FG_BUILDING6_WINDOW_BYTES, 0},
    {
        "walkstuf1",
        FG_WALKSTUF1_HIGH_RESIDUAL_WINDOW_BYTES,
        FG_WALKSTUF1_LOW_RESIDUAL_WINDOW_BYTES,
        kFgPilotPackFormatIndexed8TemporalResidual
    }
};

static const struct TFgRuntimeSetupPrimePolicy kFgRuntimeSetupPrimePolicies[] = {
    {"fishing1", FG_SETUP_PRIME_WINDOW_BYTES, FG_SETUP_PRIME_WINDOW_BYTES},
    {
        "fishing2",
        FG_FISHING2_SETUP_PRIME_WINDOW_BYTES,
        FG_FISHING2_SETUP_PRIME_WINDOW_BYTES - (96UL * 1024UL)
    },
    {"building2", 0, 128UL * 1024UL},
    {"activity12", FG_ACTIVITY12_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"fishing3", 128UL * 1024UL, FG_SETUP_PRIME_WINDOW_BYTES - (32UL * 1024UL)},
    {"fishing6", FG_FISHING6_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"fishing7", FG_FISHING7_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"johnny3", FG_JOHNNY3_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"visitor3", FG_VISITOR3_HIGH_SETUP_PRIME_WINDOW_BYTES, FG_VISITOR3_LOW_SETUP_PRIME_WINDOW_BYTES},
    {"visitor1", FG_VISITOR1_HIGH_SETUP_PRIME_WINDOW_BYTES, 0},
    {"visitor7", FG_VISITOR7_HIGH_SETUP_PRIME_WINDOW_BYTES, 0}
};

struct TFgPilotSceneFamily {
    const char *scenePrefix;
    const char *adsName;
    const char *highPackPrefix;
    const char *lowPackPrefix;
    uint32 tagMask;
};

#define FG_TAG_MASK(tag_) (1UL << (tag_))
#define FG_TAG_RANGE_1_2 (FG_TAG_MASK(1) | FG_TAG_MASK(2))
#define FG_TAG_RANGE_1_3 (FG_TAG_RANGE_1_2 | FG_TAG_MASK(3))
#define FG_TAG_RANGE_1_5 (FG_TAG_RANGE_1_3 | FG_TAG_MASK(4) | FG_TAG_MASK(5))
#define FG_TAG_RANGE_1_6 (FG_TAG_RANGE_1_5 | FG_TAG_MASK(6))
#define FG_TAG_RANGE_1_7 (FG_TAG_RANGE_1_6 | FG_TAG_MASK(7))
#define FG_TAG_RANGE_1_8 (FG_TAG_RANGE_1_7 | FG_TAG_MASK(8))

static const struct TFgPilotSceneFamily kFgPilotSceneFamilies[] = {
    { "activity", "ACTIVITY", "ACTV",    "ACTV",
      FG_TAG_MASK(1) | FG_TAG_MASK(4) | FG_TAG_MASK(5) |
      FG_TAG_MASK(6) | FG_TAG_MASK(7) | FG_TAG_MASK(8) |
      FG_TAG_MASK(9) | FG_TAG_MASK(10) | FG_TAG_MASK(11) |
      FG_TAG_MASK(12) },
    { "building", "BUILDING", "BUIL",    "BUIL", FG_TAG_RANGE_1_7 },
    { "fishing",  "FISHING",  "FISHING", "FISH", FG_TAG_RANGE_1_8 },
    { "johnny",   "JOHNNY",   "JOHNNY",  "JOHN", FG_TAG_RANGE_1_6 },
    { "mary",     "MARY",     "MARY",    "MARY", FG_TAG_RANGE_1_5 },
    { "miscgag",  "MISCGAG",  "MISCGAG", "MISC", FG_TAG_RANGE_1_2 },
    { "stand",    "STAND",    "STAND",   "STND",
      FG_TAG_RANGE_1_8 | FG_TAG_MASK(9) | FG_TAG_MASK(10) |
      FG_TAG_MASK(11) | FG_TAG_MASK(12) | FG_TAG_MASK(15) |
      FG_TAG_MASK(16) },
    { "suzy",     "SUZY",     "SUZY",    "SUZY", FG_TAG_RANGE_1_2 },
    { "visitor",  "VISITOR",  "VISITOR", "VIST",
      FG_TAG_MASK(1) | FG_TAG_MASK(3) | FG_TAG_MASK(4) |
      FG_TAG_MASK(5) | FG_TAG_MASK(6) | FG_TAG_MASK(7) },
    { "walkstuf", "WALKSTUF", "WALK",    "WALK", FG_TAG_RANGE_1_3 }
};

static char gFgCompactOverlayPackPath[24];

static const struct TFgPilotReadGroup kFishing3HighReadGroups12[] = {
    {223, 234, 0},
    {234, 246, 0},
    {345, 354, 0}
};

static const struct TFgPilotReadGroup kFishing3LowReadGroups12[] = {
    {159, 171, 0},
    {163, 175, 0},
    {253, 265, 0}
};

static const struct TFgPilotReadGroup kBuilding2HighReadGroups12[] = {
    {60, 72, 0},
    {206, 230, 0},
    {226, 242, 0},
    {83, 95, 0},
    {158, 174, 0},
    {185, 197, 0},
    {271, 287, 5},
    {315, 327, 0}
};

static const struct TFgPilotReadGroup kBuilding2LowReadGroups12[] = {
    {141, 153, 0},
    {218, 229, 8},
    {238, 250, 0},
    {318, 330, 0},
    {365, 381, 0}
};

static const struct TFgPilotReadGroup kVisitor5LowReadGroups12[] = {
    {30, 46, 0}
};

static const struct TFgPilotReadGroup kVisitor5HighReadGroups12[] = {
    {30, 46, 0}
};

static const struct TFgPilotReadGroup kVisitor3LowReadGroups16[] = {
    {16, 32, 0},
    {72, 88, 0},
    {88, 104, 0}
};

static const struct TFgPilotReadGroup kWalkstuf1HighReadGroups12[] = {
    {78, 91, 0},
    {92, 108, 0},
    {108, 124, 0},
    {149, 165, 0},
    {178, 194, 0},
    {201, 213, 0},
    {213, 229, 0},
    {272, 284, 0},
    {344, 360, 0},
    {395, 411, 0},
    {411, 423, 0},
    {404, 416, 0},
    {423, 439, 0},
    {427, 443, 0},
    {443, 455, 0},
    {444, 456, 0}
};

static const struct TFgPilotReadGroup kWalkstuf1LowReadGroups12[] = {
    {78, 91, 0},
    {91, 107, 0},
    {113, 129, 0},
    {201, 213, 0},
    {209, 225, 0},
    {213, 229, 0},
    {344, 360, 0},
    {355, 371, 0},
    {378, 390, 0},
    {394, 410, 0},
    {422, 434, 0},
    {427, 443, 0},
    {443, 455, 0},
    {444, 456, 0}
};
