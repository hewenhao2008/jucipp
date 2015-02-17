#ifndef JUCI_MENU_H_
#define JUCI_MENU_H_

#include <iostream>
#include "gtkmm.h"
#include "keybindings.h"
#include "api.h"


namespace Menu {
    class View {
    public:
      explicit View(Gtk::Orientation orient);

      Gtk::Box &view(Glib::RefPtr<Gtk::UIManager> ui_manager);

    protected:
      Gtk::Box view_;
    };  // class View

    class Controller {
    public:
      explicit Controller(Keybindings::Controller& keybindings);

      Gtk::Box &view();

    private:
      Keybindings::Controller keybindings_;
      View menu_view_;

      void OnFileNewEmptyfile();

      // Signal handlers
      void OnFileNewCCFile();

      void OnFileNewHeaderFile();

      void OnFileOpenFile();

      void OnFileOpenFolder();

      void OnPluginAddSnippet();

      void OnWindowCloseTab();

      void OnEditCopy();

      void OnEditCut();

      void OnEditPaste();

      void OnEditFind();

      void OnWindowSplitWindow();

      void OnHelpAbout();
    };  // class Controller
}  // namespace Menu
#endif  // JUCI_MENU_H_
