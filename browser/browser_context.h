// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE-CHROMIUM file.

#ifndef BRIGHTRAY_BROWSER_BROWSER_CONTEXT_H_
#define BRIGHTRAY_BROWSER_BROWSER_CONTEXT_H_

#include "browser/url_request_context_getter.h"

#include "content/public/browser/browser_context.h"
#include "browser/network_delegate.h"

class PrefRegistrySimple;
class PrefService;

namespace brightray {

class BrowserContext : public content::BrowserContext,
                       public brightray::URLRequestContextGetter::Delegate {
 public:
  BrowserContext();
  ~BrowserContext();

  virtual void Initialize();

  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector protocol_interceptors);

  net::URLRequestContextGetter* url_request_context_getter() const {
    return url_request_getter_.get();
  }

  NetworkDelegate* GetNetworkDelegate() const {
    return network_delegate_.get();
  }

  PrefService* prefs() { return prefs_.get(); }

 protected:
  // Subclasses should override this to register custom preferences.
  virtual void RegisterPrefs(PrefRegistrySimple* pref_registry) {}

  // URLRequestContextGetter::Delegate:
  virtual net::NetworkDelegate* CreateNetworkDelegate() override;

  virtual base::FilePath GetPath() const override;

 private:
  class ResourceContext;

  void RegisterInternalPrefs(PrefRegistrySimple* pref_registry);

  scoped_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
      const base::FilePath& partition_path) override;
  bool IsOffTheRecord() const override;
  net::URLRequestContextGetter* GetRequestContext() override;
  net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id);
  net::URLRequestContextGetter* GetMediaRequestContext() override;
  net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) override;
  net::URLRequestContextGetter* GetMediaRequestContextForStoragePartition(
      const base::FilePath& partition_path, bool in_memory) override;
  content::ResourceContext* GetResourceContext() override;
  content::DownloadManagerDelegate* GetDownloadManagerDelegate() override;
  content::BrowserPluginGuestManager* GetGuestManager() override;
  storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
  content::PushMessagingService* GetPushMessagingService() override;
  content::SSLHostStateDelegate* GetSSLHostStateDelegate() override;

  base::FilePath path_;
  scoped_ptr<NetworkDelegate> network_delegate_;  
  scoped_ptr<ResourceContext> resource_context_;
  scoped_refptr<URLRequestContextGetter> url_request_getter_;
  scoped_ptr<PrefService> prefs_;

  DISALLOW_COPY_AND_ASSIGN(BrowserContext);
};

}  // namespace brightray

#endif
