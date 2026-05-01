#ifndef SIRIUS_THEME_H
#define SIRIUS_THEME_H

struct ImFont;

extern ImFont* g_FontMain;
extern ImFont* g_FontMono;

void LoadFonts();
void ApplyDarkTheme();

bool PrimaryButton(const char* label);
bool DangerButton(const char* label);

#endif
