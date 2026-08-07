#include <universal/q_shared.h>
#include "snd_local.h"
#include "snd_public.h"

//    char const **snd_eqTypeStrings 827c07a8     snd_utils.obj
//    char const **snd_roomStrings  827c07c0     snd_utils.obj

const char *snd_eqTypeStrings[6] = { "lowpass", "highpass", "lowshelf", "highshelf", "bell", NULL };
const char *snd_roomStrings[27] =
{
  "generic",
  "paddedcell",
  "room",
  "bathroom",
  "livingroom",
  "stoneroom",
  "auditorium",
  "concerthall",
  "cave",
  "arena",
  "hangar",
  "carpetedhallway",
  "hallway",
  "stonecorridor",
  "alley",
  "forest",
  "city",
  "mountains",
  "quarry",
  "plain",
  "parkinglot",
  "sewerpipe",
  "underwater",
  "drugged",
  "dizzy",
  "psychotic",
  NULL
};

