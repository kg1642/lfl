/*
 * $Id: new_app_template.cpp 1305 2014-09-02 08:10:25Z justin $
 * Copyright (C) 2009 Lucid Fusion Labs

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lfapp/lfapp.h"
#include "lfapp/dom.h"
#include "lfapp/css.h"
#include "lfapp/gui.h"

namespace LFL {
BindMap binds;
AssetMap asset;
SoundAssetMap soundasset;
Scene scene;
EditorDialog *editor; 

int Frame(LFL::Window *W, unsigned clicks, unsigned mic_samples, bool cam_sample, int flag) {
    screen->gd->DrawMode(DrawMode::_2D);
    screen->DrawDialogs();
    return 0;
}

}; // namespace LFL
using namespace LFL;

extern "C" int main(int argc, const char *argv[]) {

    app->logfilename = StrCat(dldir(), "editor.txt");
    app->frame_cb = Frame;
    screen->width = 840;
    screen->height = 760;
    screen->caption = "Editor";

    if (app->Create(argc, argv, __FILE__)) { app->Free(); return -1; }
    if (app->Init()) { app->Free(); return -1; }

    // binds.push_back(Bind(key,         callback));
    binds.push_back(Bind(Key::Backquote, Bind::CB(bind([&]() { screen->console->Toggle(); }))));
    binds.push_back(Bind(Key::Quote,     Bind::CB(bind([&]() { screen->console->Toggle(); }))));
    binds.push_back(Bind(Key::Escape,    Bind::CB(bind(&Shell::quit, &app->shell, vector<string>()))));
    screen->binds = &binds;

    string s = LocalFile::FileContents(StrCat(ASSETS_DIR, "lfapp_vertex.glsl"));
    editor = new EditorDialog(screen, Fonts::Default(), new BufferFile(s.c_str(), s.size()), 1, 1);

    // start our engine
    return app->Main();
}