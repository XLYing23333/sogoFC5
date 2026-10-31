#include <cstdlib>
#include <new>

// The vendor expects this standard exception helper under Qt_5. Noble Qt
// delegates the same exception to libstdc++ without exporting that alias.
extern "C" [[noreturn]] void sogo_bad_array_length() asm("__cxa_throw_bad_array_new_length");
extern "C" [[noreturn]] void sogo_bad_array_length(){throw std::bad_array_new_length();}

// MainWindow's vtable belongs to the original GUI executable, not its shared
// library. No instance is permitted in this Core-only process. Reject both
// constructor entry points before QWidget can connect to the desktop. The
// 512-byte relocation target is deliberately not a usable window vtable.
extern "C" {
void* sogo_forbidden_window_table[64] asm("_ZTVN6n_sgxx10MainWindowE") = {};
[[noreturn]] void sogo_forbidden_window_c1(void*,void*,void*) asm("_ZN6n_sgxx10MainWindowC1EP7QWidgetPNS_16t_wndTopImplBaseE");
[[noreturn]] void sogo_forbidden_window_c2(void*,void*,void*) asm("_ZN6n_sgxx10MainWindowC2EP7QWidgetPNS_16t_wndTopImplBaseE");
[[noreturn]] void sogo_forbidden_window_c1(void*,void*,void*){std::abort();}
[[noreturn]] void sogo_forbidden_window_c2(void*,void*,void*){std::abort();}

// Disable imported input-injection and keyboard-grab APIs inside the worker.
int XTestFakeKeyEvent(void*,unsigned int,int,unsigned long){return 0;}
int XTestFakeButtonEvent(void*,unsigned int,int,unsigned long){return 0;}
int XGrabKeyboard(void*,unsigned long,int,int,int,unsigned long){return 1;}
}
