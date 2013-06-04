#ifndef BRIGHTRAY_BROWSER_WIN_INSPECTABLE_WEB_CONTENTS_VIEW_WIN_H_
#define BRIGHTRAY_BROWSER_WIN_INSPECTABLE_WEB_CONTENTS_VIEW_WIN_H_

#include "browser/inspectable_web_contents_view.h"

#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"

namespace brightray {

class DevToolsWindow;
class InspectableWebContentsImpl;

class InspectableWebContentsViewWin : public InspectableWebContentsView {
public:
  InspectableWebContentsViewWin(InspectableWebContentsImpl*);
  ~InspectableWebContentsViewWin();

  virtual gfx::NativeView GetNativeView() const OVERRIDE;
  virtual void ShowDevTools() OVERRIDE;
  virtual void CloseDevTools() OVERRIDE;
  virtual bool SetDockSide(const std::string& side) OVERRIDE;

  InspectableWebContentsImpl* inspectable_web_contents() { return inspectable_web_contents_; }

private:
  // Owns us.
  InspectableWebContentsImpl* inspectable_web_contents_;

  base::WeakPtr<DevToolsWindow> devtools_window_;

  DISALLOW_COPY_AND_ASSIGN(InspectableWebContentsViewWin);
};

}

#endif
