#pragma once

#ifndef NON_SERIALIZED
// Add this before a variable to tell EHT to not generate serialization code for it.
// Add this before a class to tell EHT to not generaty any serialization code for the entire class.
#define NON_SERIALIZED
#endif

#ifndef CUSTOM_EDITOR
// Add this before a variable to tell EHT to not generate any editor code for it.
#define CUSTOM_EDITOR
#endif

#ifndef CUSTOM_EDITOR_ONLY
// Add this before a class to tell EHT to not generaty any editor code for the entire class,
// with the exception of the draw_editor() function itself and a call to the custom editor.
#define CUSTOM_EDITOR_ONLY
#endif
