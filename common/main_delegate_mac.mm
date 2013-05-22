// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Adam Roben <adam@roben.org>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE-CHROMIUM file.

#import "main_delegate.h"

#include "common/application_info.h"
#include "common/mac/foundation_util.h"
#include "common/mac/main_application_bundle.h"

#include "base/mac/bundle_locations.h"
#include "base/path_service.h"
#include "content/public/common/content_paths.h"

namespace brightray {

namespace {

base::FilePath GetFrameworksPath() {
  return MainApplicationBundlePath().Append("Contents").Append("Frameworks");
}

}

base::FilePath MainDelegate::GetResourcesPakFilePath() {
  auto path = [base::mac::FrameworkBundle() pathForResource:@"content_shell" ofType:@"pak"];
  return base::mac::NSStringToFilePath(path);
}

void MainDelegate::OverrideFrameworkBundlePath() {
  base::FilePath helper_path = GetFrameworksPath().Append(GetApplicationName() + ".framework");

  base::mac::SetOverrideFrameworkBundlePath(helper_path);
}

void MainDelegate::OverrideChildProcessPath() {
  base::FilePath helper_path = GetFrameworksPath().Append(GetApplicationName() + " Helper.app")
    .Append("Contents")
    .Append("MacOS")
    .Append(GetApplicationName() + " Helper");

  PathService::Override(content::CHILD_PROCESS_EXE, helper_path);
}

}
