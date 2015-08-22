// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE-CHROMIUM file.

#include "browser/browser_client.h"

#include "browser/browser_context.h"
#include "browser/browser_main_parts.h"
#include "browser/devtools_manager_delegate.h"
#include "browser/media/media_capture_devices_dispatcher.h"
#include "browser/platform_notification_service_impl.h"

#include "base/path_service.h"
#include "content/public/common/url_constants.h"

namespace brightray {

namespace {

BrowserClient* g_browser_client;

}

BrowserClient* BrowserClient::Get() {
  return g_browser_client;
}

BrowserClient::BrowserClient()
    : browser_main_parts_(nullptr) {
  DCHECK(!g_browser_client);
  g_browser_client = this;
}

BrowserClient::~BrowserClient() {
}

BrowserContext* BrowserClient::browser_context() {
  return browser_main_parts_->browser_context();
}

BrowserMainParts* BrowserClient::OverrideCreateBrowserMainParts(
    const content::MainFunctionParams&) {
  return new BrowserMainParts;
}

content::BrowserMainParts* BrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
  DCHECK(!browser_main_parts_);
  browser_main_parts_ = OverrideCreateBrowserMainParts(parameters);
  return browser_main_parts_;
}

net::URLRequestContextGetter* BrowserClient::CreateRequestContext(
    content::BrowserContext* browser_context,
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector protocol_interceptors) {
  auto context = static_cast<BrowserContext*>(browser_context);
  return context->CreateRequestContext(static_cast<NetLog*>(GetNetLog()),
                                       protocol_handlers,
                                       protocol_interceptors.Pass());
}

net::URLRequestContextGetter* BrowserClient::CreateRequestContextForStoragePartition(
    content::BrowserContext* browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector protocol_interceptors) {
  auto context = static_cast<BrowserContext*>(browser_context);
  return context->CreateRequestContextForStoragePartition(partition_path,
                                                          in_memory,
                                                          protocol_handlers,
                                                          protocol_interceptors.Pass());
}

content::MediaObserver* BrowserClient::GetMediaObserver() {
  return MediaCaptureDevicesDispatcher::GetInstance();
}

content::PlatformNotificationService* BrowserClient::GetPlatformNotificationService() {
  return PlatformNotificationServiceImpl::GetInstance();
}

void BrowserClient::GetAdditionalAllowedSchemesForFileSystem(
    std::vector<std::string>* additional_schemes) {
  additional_schemes->push_back(content::kChromeDevToolsScheme);
  additional_schemes->push_back(content::kChromeUIScheme);
}

net::NetLog* BrowserClient::GetNetLog() {
  return &net_log_;
}

base::FilePath BrowserClient::GetDefaultDownloadDirectory() {
  // ~/Downloads
  base::FilePath path;
  if (PathService::Get(base::DIR_HOME, &path))
    path = path.Append(FILE_PATH_LITERAL("Downloads"));

  return path;
}

content::DevToolsManagerDelegate* BrowserClient::GetDevToolsManagerDelegate() {
  return new DevToolsManagerDelegate;
}

}  // namespace brightray
