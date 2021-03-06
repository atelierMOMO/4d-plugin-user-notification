4d-plugin-user-notification
===========================

Send Mac OS X Mountain Lion Local User Notification from 4D.

### Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
||<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|||

### Version

<img src="https://cloud.githubusercontent.com/assets/1725068/18940649/21945000-8645-11e6-86ed-4a0f800e5a73.png" width="32" height="32" /> <img src="https://cloud.githubusercontent.com/assets/1725068/18940648/2192ddba-8645-11e6-864d-6d5692d55717.png" width="32" height="32" /> <img src="https://user-images.githubusercontent.com/1725068/41266195-ddf767b2-6e30-11e8-9d6b-2adf6a9f57a5.png" width="32" height="32" />

### Releases

[2.4](https://github.com/miyako/4d-plugin-user-notification/releases/tag/2.4)

![preemption xx](https://user-images.githubusercontent.com/1725068/41327179-4e839948-6efd-11e8-982b-a670d511e04f.png)

except 

* ``NOTIFICATION SET METHOD``

## Examples

```
NOTIFICATION SET METHOD ("notify")  //receives 6 text arguments

$title:="title"
$subtitle:="subtitle"
$informativeText:="informativeText"

NOTIFICATION SET MODE (Notification system decides)  //default
NOTIFICATION SET MODE (Notification dislay always)

$soundName:=""  //no sound
$soundName:=Notification default sound

DELIVER NOTIFICATION (\
$title;\
$subtitle;\
$informativeText;\
$soundName;\
$userInfo;"action";"close")
```
