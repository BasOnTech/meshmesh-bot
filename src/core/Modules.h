#pragma once
// Optional modules: both boards here build them all. The host UI
// preview (MM_UI_PREVIEW) can still leave one out with MM_NO_CHESS, MM_NO_PET, MM_NO_DICE, MM_NO_WARDRIVE,
// MM_NO_CHECKERS or MM_NO_SEA.
// A module left out keeps a small stand-in (its USB command answers "ERR ... is not in this build",
// the radio passes it nothing); its screens and menu items are not built. status lists the modules
// ("modules"), so the web page and the app hide what is missing.
#if defined(MM_NO_CHESS)
#define MM_CHESS 0
#else
#define MM_CHESS 1
#endif
#if defined(MM_NO_PET)
#define MM_PET 0
#else
#define MM_PET 1
#endif
#if defined(MM_NO_DICE)
#define MM_DICE 0
#else
#define MM_DICE 1
#endif
#if defined(MM_NO_WARDRIVE)
#define MM_WARDRIVE 0
#else
#define MM_WARDRIVE 1
#endif
// Checkers and sea battle (Duel.h): MM_NO_CHECKERS, MM_NO_SEA; the shared network code is built with either.
#if defined(MM_NO_CHECKERS)
#define MM_CHECKERS 0
#else
#define MM_CHECKERS 1
#endif
#if defined(MM_NO_SEA)
#define MM_SEA 0
#else
#define MM_SEA 1
#endif
#define MM_DUEL (MM_CHECKERS || MM_SEA)
#if (!MM_CHESS || !MM_PET || !MM_DICE || !MM_WARDRIVE || !MM_CHECKERS || !MM_SEA) && !defined(MM_UI_PREVIEW)
#error "Optional modules can be left out only in the host UI preview"
#endif
