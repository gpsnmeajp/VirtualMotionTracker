# VMT - VirtualMotionTracker Manual
## Caution
Please remove/uninstall old driver before new driver install.  

## How it works
**VMT**  
C++ OpenVR Driver. it receives OSC Protocol.  
  
**VMT Manager**  
C# Management tool.  
Installation, Uninstallation, Room setup.  
  
## Installation
**1. Download and extract VMT**  
[Download](https://github.com/gpsnmeajp/VirtualMotionTracker/releases)  

**2. Launch vmt_manager.exe**  

**3. Allow vmt_manager.exe in firewall**  
<img src="screen1A.png" height="300px"></img>

**4. Click "Install" button**  
Driver path registration for VR System.  
<img src="screen0.png"></img>
<img src="screen1B.png"></img>

**5. Restart SteamVR**  
vmt manager will close automatically.  
  
**6. Allow SteamVR(vrserver.exe) in firewall**  
  
## Room Matrix setup
**1. Launch vmt_manager.exe**  

**2. Start VR HMD and Controller**  
vmt_manager will get room info.  
Please wait for Room Matrix turns green.   
<img src="screen1.png"></img>
<img src="screen2.png"></img>  

**3. Click "Set Room Matrix" button.**  
Room Matrix will save in setting.json.  

## Check
**1. Click "Check VMT_0 Position" Button**  
**2. if SteamVR shows tracker, and "VMT_0 Room Position" green, it's ok.**  
if "VMT_0 Room Position" is red, please retry Room Matrix setup.  
<img src="screen2A.png"></img>
<img src="screen3.png"></img>  

## Test controller input
Please set tracker role before you test controller input.  
<img src="screen2C.png"></img>  

## Configure handheld mode.
If you want to tracker works like Controller or not, please below setting.  
**1. Click "Show all" in Manager**  
<img src="screen2B.png"></img>  
  
**2. SteamVRの設定→デバイス→Viveトラッカーを管理**  
<img src="screen4.png" height="300px"></img>  
  
**3. Viveトラッカーの管理**  
<img src="screen5.png" height="300px"></img>  
  
**4. Set Tracker roles**  
<img src="screen6.png" height="300px"></img>
<img src="screen7.png" height="300px"></img>  

## OSC Protocol

|Direction|Port number|
|---|---|
|App → Driver| 39570|
|Manager → Driver| 39570|
|Manager ← Driver| 39571|

Caution: If Port 39571 used be another application, Manager won't works.  
  
### Virtual tracker control
**Argument**  

|Args|type|detail|
|---|---|---|
|index|int| ID number。0-57|
|enable|int| 0=Disable, 1=Enable(Tracker), 2=Enable(Left Hand Controller), 3=Enable(Right Hand Controller) |
|timeoffset|float| TimeOffset, normaly 0|
|x,y,z|float| Position|
|qx,qy,qz,qw|float| Rotaion(Quaternion)|

***Type(Tracker or Controller) only works in first registration time.***  

**/VMT/Room/Unity index, enable, timeoffset, x, y, z, qx, qy, qz, qw**  
Unity lik Left-handed space, and Room space. (Recommended)  
  
**/VMT/Room/Driver index, enable, timeoffset, x, y, z, qx, qy, qz, qw**  
Driver Right-handed space, and Room space.  
  
**/VMT/Raw/Unity index, enable, timeoffset, x, y, z, qx, qy, qz, qw**  
Unity lik Left-handed space, and Driver space.  
  
**/VMT/Raw/Driver index, enable, timeoffset, x, y, z, qx, qy, qz, qw**  
Driver Right-handed space, and Driver space.  
  
### Input
|Method|Range|
|---|---|
|ButtonIndex(int)| 0～7|
|TriggerIndex(int)| 0, 1|
|JoyStickIndex(int)| 0|


**/VMT/Input/Button index, buttonindex, timeoffset, value**  
Button input.  
value(int):1=press, 0=Release  
  
**/VMT/Input/Trigger index, triggerindex, timeoffset, value**  
Trigger input.  
value(float):0.0 ～ 1.0  

**/VMT/Input/Joystick index, joystickindex, timeoffset, x, y**  
Joystick input.  
x,y(float):-1.0 ～ 1.0  
  
### Driver Management
**/VMT/Reset**  
All tracker will not-tracking state.  
  
**/VMT/LoadSetting**  
Reload json.  
  
**/VMT/SetRoomMatrix m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12**  
Set and Save Room Matrix.  
Please do not send periodic. it writes setting on drive.    

**/VMT/SetRoomMatrix/Temporary m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12**  
Set Room Matrix temporary.  
It is volatile.    

### Drivers response  
**/VMT/Out/Log stat,msg**  
stat(int): Status(0=info,1=warn,2=err)  
msg(string): Message  
  
**/VMT/Out/Alive version, installpath**  
version(string): Version  
installpath(string): Driver installed path
  
**/VMT/Out/Haptic index, frequency, amplitude, duration**  
frequency(float): frequency  
amplitude(float): amplitude  
duration(float): duration  

### Unity sample
Plese use [hecomi/uOSC](https://github.com/hecomi/uOSC)  
Send gameobject transform to virtual tracker.  
  
```cs
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
public class sendme : MonoBehaviour
{
    uOSC.uOscClient client;
    void Start()
    {
        client = GetComponent<uOSC.uOscClient>();
    }

    void Update()
    {
        client.Send("/VMT/Room/Unity", (int)0, (int)1, (float)0f,
            (float)transform.position.x,
            (float)transform.position.y,
            (float)transform.position.z,
            (float)transform.rotation.x,
            (float)transform.rotation.y,
            (float)transform.rotation.z,
            (float)transform.rotation.w
        );
    }
}
```
## Commandline
|Example|function|
|---|---|
|vmt_manager.exe install|Install driver|
|vmt_manager.exe uninstall|Uninstall driver|

## Tracking Override
https://github.com/ValveSoftware/openvr/wiki/TrackingOverrides

steamvr.vrsettings

```
   "TrackingOverrides" : {
      "/devices/vmt/VMT_0" : "/user/head",
      "/devices/vmt/VMT_1" : "/user/hand/left",
      "/devices/vmt/VMT_2" : "/user/hand/right"
   },
```

