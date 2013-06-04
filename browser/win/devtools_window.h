#ifndef BRIGHTRAY_BROWSER_WIN_DEVTOOLS_WINDOW_H_
#define BRIGHTRAY_BROWSER_WIN_DEVTOOLS_WINDOW_H_

#include "base/memory/weak_ptr.h"
#include "ui/base/win/window_impl.h"

namespace brightray {

class InspectableWebContentsViewWin;

class DevToolsWindow : public ui::WindowImpl, public base::SupportsWeakPtr<DevToolsWindow> {
 public:
  static DevToolsWindow* Create(InspectableWebContentsViewWin*);

  BEGIN_MSG_MAP_EX(DevToolsWindow)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
  END_MSG_MAP()

 private:
  DevToolsWindow(InspectableWebContentsViewWin*);
  ~DevToolsWindow();

  LRESULT OnCreate(UINT message, WPARAM, LPARAM, BOOL& handled);
  LRESULT OnDestroy(UINT message, WPARAM, LPARAM, BOOL& handled);
  LRESULT OnSize(UINT message, WPARAM, LPARAM, BOOL& handled);

  InspectableWebContentsViewWin* controller_;

  DISALLOW_COPY_AND_ASSIGN(DevToolsWindow);
};

}

#endif
