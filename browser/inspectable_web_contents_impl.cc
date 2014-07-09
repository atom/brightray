// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Adam Roben <adam@roben.org>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE-CHROMIUM file.

#include "browser/inspectable_web_contents_impl.h"

#include "browser/browser_client.h"
#include "browser/browser_context.h"
#include "browser/browser_main_parts.h"
#include "browser/inspectable_web_contents_delegate.h"
#include "browser/inspectable_web_contents_view.h"

#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_client_host.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/devtools_manager.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/browser/render_view_host.h"

namespace brightray {

namespace {

const char kChromeUIDevToolsURL[] = "chrome-devtools://devtools/devtools.html?can_dock=true";
const char kDevToolsBoundsPref[] = "brightray.devtools.bounds";

void RectToDictionary(const gfx::Rect& bounds, base::DictionaryValue* dict) {
  dict->SetInteger("x", bounds.x());
  dict->SetInteger("y", bounds.y());
  dict->SetInteger("width", bounds.width());
  dict->SetInteger("height", bounds.height());
}

void DictionaryToRect(const base::DictionaryValue& dict, gfx::Rect* bounds) {
  int x = 0, y = 0, width = 800, height = 600;
  dict.GetInteger("x", &x);
  dict.GetInteger("y", &y);
  dict.GetInteger("width", &width);
  dict.GetInteger("height", &height);
  *bounds = gfx::Rect(x, y, width, height);
}

}  // namespace

// Implemented separately on each platform.
InspectableWebContentsView* CreateInspectableContentsView(
    InspectableWebContentsImpl* inspectable_web_contents_impl);

void InspectableWebContentsImpl::RegisterPrefs(PrefRegistrySimple* registry) {
  scoped_ptr<base::DictionaryValue> bounds_dict(new base::DictionaryValue);
  RectToDictionary(gfx::Rect(0, 0, 800, 600), bounds_dict.get());
  registry->RegisterDictionaryPref(kDevToolsBoundsPref, bounds_dict.release());
}

InspectableWebContentsImpl::InspectableWebContentsImpl(
    content::WebContents* web_contents)
    : web_contents_(web_contents),
      delegate_(nullptr) {
  auto context = static_cast<BrowserContext*>(web_contents_->GetBrowserContext());
  auto bounds_dict = context->prefs()->GetDictionary(kDevToolsBoundsPref);
  if (bounds_dict)
    DictionaryToRect(*bounds_dict, &devtools_bounds_);

  view_.reset(CreateInspectableContentsView(this));
}

InspectableWebContentsImpl::~InspectableWebContentsImpl() {
}

InspectableWebContentsView* InspectableWebContentsImpl::GetView() const {
  return view_.get();
}

content::WebContents* InspectableWebContentsImpl::GetWebContents() const {
  return web_contents_.get();
}

void InspectableWebContentsImpl::ShowDevTools() {
  // Show devtools only after it has done loading, this is to make sure the
  // SetIsDocked is called *BEFORE* ShowDevTools.
  if (!devtools_web_contents_) {
    embedder_message_dispatcher_.reset(
        new DevToolsEmbedderMessageDispatcher(this));

    auto create_params = content::WebContents::CreateParams(
        web_contents_->GetBrowserContext());
    devtools_web_contents_.reset(content::WebContents::Create(create_params));

    Observe(devtools_web_contents_.get());
    devtools_web_contents_->SetDelegate(this);

    agent_host_ = content::DevToolsAgentHost::GetOrCreateFor(
        web_contents_->GetRenderViewHost());
    frontend_host_.reset(
        content::DevToolsClientHost::CreateDevToolsFrontendHost(
            devtools_web_contents_.get(), this));
    content::DevToolsManager::GetInstance()->RegisterDevToolsClientHostFor(
        agent_host_, frontend_host_.get());

    GURL devtools_url(kChromeUIDevToolsURL);
    devtools_web_contents_->GetController().LoadURL(
        devtools_url,
        content::Referrer(),
        content::PAGE_TRANSITION_AUTO_TOPLEVEL,
        std::string());
  } else {
    view_->ShowDevTools();
  }
}

void InspectableWebContentsImpl::CloseDevTools() {
  if (devtools_web_contents_) {
    view_->CloseDevTools();
    devtools_web_contents_.reset();
    web_contents_->GetView()->Focus();
  }
}

bool InspectableWebContentsImpl::IsDevToolsViewShowing() {
  return devtools_web_contents_ && view_->IsDevToolsViewShowing();
}

gfx::Rect InspectableWebContentsImpl::GetDevToolsBounds() const {
  return devtools_bounds_;
}

void InspectableWebContentsImpl::SaveDevToolsBounds(const gfx::Rect& bounds) {
  auto context = static_cast<BrowserContext*>(web_contents_->GetBrowserContext());
  base::DictionaryValue bounds_dict;
  RectToDictionary(bounds, &bounds_dict);
  context->prefs()->Set(kDevToolsBoundsPref, bounds_dict);
  devtools_bounds_ = bounds;
}

void InspectableWebContentsImpl::ActivateWindow() {
}

void InspectableWebContentsImpl::CloseWindow() {
  CloseDevTools();
}

void InspectableWebContentsImpl::SetContentsResizingStrategy(
    const gfx::Insets& insets, const gfx::Size& min_size) {
  DevToolsContentsResizingStrategy strategy(insets, min_size);
  if (contents_resizing_strategy_.Equals(strategy))
    return;

  contents_resizing_strategy_.CopyFrom(strategy);
  view_->SetContentsResizingStrategy(contents_resizing_strategy_);
}

void InspectableWebContentsImpl::InspectElementCompleted() {
}

void InspectableWebContentsImpl::MoveWindow(int x, int y) {
}

void InspectableWebContentsImpl::SetIsDocked(bool docked) {
  view_->SetIsDocked(docked);
}

void InspectableWebContentsImpl::OpenInNewTab(const std::string& url) {
}

void InspectableWebContentsImpl::SaveToFile(
    const std::string& url, const std::string& content, bool save_as) {
  if (delegate_)
    delegate_->DevToolsSaveToFile(url, content, save_as);
}

void InspectableWebContentsImpl::AppendToFile(
    const std::string& url, const std::string& content) {
  if (delegate_)
    delegate_->DevToolsAppendToFile(url, content);
}

void InspectableWebContentsImpl::RequestFileSystems() {
}

void InspectableWebContentsImpl::AddFileSystem() {
}

void InspectableWebContentsImpl::RemoveFileSystem(
    const std::string& file_system_path) {
}

void InspectableWebContentsImpl::UpgradeDraggedFileSystemPermissions(
    const std::string& file_system_url) {
}

void InspectableWebContentsImpl::IndexPath(
    int request_id, const std::string& file_system_path) {
}

void InspectableWebContentsImpl::StopIndexing(int request_id) {
}

void InspectableWebContentsImpl::SearchInPath(
    int request_id,
    const std::string& file_system_path,
    const std::string& query) {
}

void InspectableWebContentsImpl::ZoomIn() {
}

void InspectableWebContentsImpl::ZoomOut() {
}

void InspectableWebContentsImpl::ResetZoom() {
}

void InspectableWebContentsImpl::DispatchOnEmbedder(
    const std::string& message) {
  embedder_message_dispatcher_->Dispatch(message);
}

void InspectableWebContentsImpl::InspectedContentsClosing() {
}

void InspectableWebContentsImpl::AboutToNavigateRenderView(
    content::RenderViewHost* render_view_host) {
  content::DevToolsClientHost::SetupDevToolsFrontendClient(
      web_contents()->GetRenderViewHost());
}

void InspectableWebContentsImpl::DidFinishLoad(int64 frame_id,
                                               const GURL& validated_url,
                                               bool is_main_frame,
                                               content::RenderViewHost*) {
  if (!is_main_frame)
    return;

  view_->ShowDevTools();
}

void InspectableWebContentsImpl::WebContentsDestroyed(content::WebContents*) {
  content::DevToolsManager::GetInstance()->ClientHostClosing(
      frontend_host_.get());
  Observe(nullptr);
  agent_host_ = nullptr;
  frontend_host_.reset();
}

void InspectableWebContentsImpl::HandleKeyboardEvent(
    content::WebContents* source,
    const content::NativeWebKeyboardEvent& event) {
  auto delegate = web_contents_->GetDelegate();
  if (delegate)
    delegate->HandleKeyboardEvent(source, event);
}

}  // namespace brightray
