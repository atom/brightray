// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "browser/linux/libnotify_notification.h"

#include "base/files/file_enumerator.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "browser/notification_delegate.h"
#include "chrome/browser/ui/libgtk2ui/skia_utils_gtk2.h"
#include "common/application_info.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace brightray {

namespace {
LibNotifyLoader libnotify_loader_;

bool NotifyOSDIsRunning() {
  if (getenv("ELECTRON_USE_UBUNTU_NOTIFIER"))
    return true;

  static bool notify_has_result = false;
  static bool notify_result = false;
  char *notifier_name = NULL;

  if (notify_has_result)
    return notify_result;

  if (libnotify_loader_.notify_get_server_info(&notifier_name, NULL, NULL, NULL)) {
    notify_has_result = true;

    if (g_strcmp0(notifier_name, "notify-osd") == 0) {
      notify_result = true;
    }
  }

  g_free(notifier_name);
  return notify_result;
}

void log_and_clear_error(GError* error, const char* context) {
  LOG(ERROR) << context
             << ": domain=" << error->domain
             << " code=" << error->code
             << " message=\"" << error->message << '"';
  g_error_free(error);
}

}  // namespace

// static
Notification* Notification::Create(NotificationDelegate* delegate,
                                   NotificationPresenter* presenter) {
  return new LibnotifyNotification(delegate, presenter);
}

// static
bool LibnotifyNotification::Initialize() {
  if (!libnotify_loader_.Load("libnotify.so.4") &&
      !libnotify_loader_.Load("libnotify.so.1") &&
      !libnotify_loader_.Load("libnotify.so")) {
    return false;
  }
  if (!libnotify_loader_.notify_is_initted() &&
      !libnotify_loader_.notify_init(GetApplicationName().c_str())) {
    return false;
  }
  return true;
}

LibnotifyNotification::LibnotifyNotification(NotificationDelegate* delegate,
                                             NotificationPresenter* presenter)
    : Notification(delegate, presenter),
      notification_(nullptr) {
}

LibnotifyNotification::~LibnotifyNotification() {
  g_signal_handlers_disconnect_by_data(notification_, this);
  g_object_unref(notification_);
}

void LibnotifyNotification::Show(const base::string16& title,
                                 const base::string16& body,
                                 const GURL& icon_url,
                                 const SkBitmap& icon,
                                 const bool silent) {
  notification_ = libnotify_loader_.notify_notification_new(
      base::UTF16ToUTF8(title).c_str(),
      base::UTF16ToUTF8(body).c_str(),
      nullptr);

  g_signal_connect(
      notification_, "closed", G_CALLBACK(OnNotificationClosedThunk), this);

  // NB: On Unity and on any other DE using Notify-OSD, adding a notification
  // action will cause the notification to display as a modal dialog box.
  if (!NotifyOSDIsRunning()) {
    libnotify_loader_.notify_notification_add_action(
        notification_, "default", "View", OnNotificationViewThunk, this,
        nullptr);
  }

  if (!icon.drawsNothing()) {
    GdkPixbuf* pixbuf = libgtk2ui::GdkPixbufFromSkBitmap(icon);
    libnotify_loader_.notify_notification_set_image_from_pixbuf(
        notification_, pixbuf);
    libnotify_loader_.notify_notification_set_timeout(
        notification_, NOTIFY_EXPIRES_DEFAULT);
    g_object_unref(pixbuf);
  }

  GError* error = nullptr;
  libnotify_loader_.notify_notification_show(notification_, &error);
  if (error) {
    log_and_clear_error(error, "notify_notification_show");
    NotificationFailed();
    return;
  }

  delegate()->NotificationDisplayed();
}

void LibnotifyNotification::Dismiss() {
  GError* error = nullptr;
  libnotify_loader_.notify_notification_close(notification_, &error);
  if (error) {
    log_and_clear_error(error, "notify_notification_close");
    Destroy();
  }
}

void LibnotifyNotification::OnNotificationClosed(
    NotifyNotification* notification) {
  delegate()->NotificationClosed();
  Destroy();
}

void LibnotifyNotification::OnNotificationView(
    NotifyNotification* notification, char* action) {
  delegate()->NotificationClick();
  Destroy();
}

void LibnotifyNotification::NotificationFailed() {
  delegate()->NotificationFailed();
  Destroy();
}

}  // namespace brightray
