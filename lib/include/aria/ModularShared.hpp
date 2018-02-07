#ifndef ARIA_MODULAR_SHARED_HPP
#define ARIA_MODULAR_SHARED_HPP

// Macros that are shared across modular interfaces

// ARIA_NOEXCEPT becomes noexcept, which means that the app will be immediately terminated
// if an exception is returned by this method. This is to prevent mismatches between STL used
// in Aria and STL used in the implementation of the modular interface.
// Make it platform-specific if needed in the future
#define ARIA_NOEXCEPT noexcept

#endif  //  !ARIA_MODULAR_SHARED_HPP
