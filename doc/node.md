# VMT - VirtualMotionTracker 説明書

## ドライバのインストール
**1. VMTをダウンロードし、解凍します。**  
[ダウンロード](https://github.com/gpsnmeajp/VirtualMotionTracker/releases)  
次にインストールしますので、今後移動しない場所においてください。  
デスクトップなど一時的な場所には置かないでください。  

**2. vmt_manager.exeを起動します。**  
vmt_managerフォルダ内にあります。

**3. vmt_managerをファイアーウォールで許可します。**  
お使いの環境によりますが許可してください。  

**4. Installボタンを押してください**  
ドライバーのパスがVRシステムに登録されます。

**5. SteamVRを再起動してください。**  
vmt_managerが自動で終了します。

**6. SteamVRをファイアーウォールで許可します**

## ルームセットアップ
**1. vmt_managerを起動してください。**  

**2. VR機器やコントローラーを起動してください。**  
ルーム情報を認識します。Room Matrixが緑色になるまで待ってください。  
<img src="screen1.png" height="300px"></img>
<img src="screen2.png" height="300px"></img>  

**3. Set Room Matrixボタンを押してください**  
ルーム座標変換行列が登録され、setting.jsonに保存されます。

## 動作確認
**1. Check VMT_0 Positionボタンを押してください**  
**2. SteamVRにトラッカーが表示され、VMT_0 Room Positionが緑色になればOKです**  
赤色になる場合は、Room Matrixが設定されていないか、ルーム情報が前回設定されたものと変わっています。  
<img src="screen3.png"></img>  

## OSCプロトコル
### 仮想トラッカー制御
**仮想トラッカーの引数**
|識別子|型|内容|
|---|---|---|
|index|int| 識別番号。現在0～63まで利用できます。|
|enable|int| 有効可否。1で有効、0で無効(非接続・非トラッキング状態)|
|x,y,z|float| 座標|
|qx,qy,qz,qw|float| 回転(クォータニオン)|
|timeoffset|float| 補正時間。通常0です。|

**/TrackerPoseRoomUnity index,enable,x,y,z,qx,qy,qz,qw,timeoffset**  
Unityと同じ左手系、かつ、ルーム空間(ルーム空間変換あり)で仮想トラッカーを操作します。  
通常はこれを使用します。  
  
**/TrackerPoseRoomDriver index,enable,x,y,z,qx,qy,qz,qw,timeoffset**  
OpenVRの右手系、かつ、ルーム空間(ルーム空間変換あり)で仮想トラッカーを操作します。  
  
**/TrackerPoseRawUnity index,enable,x,y,z,qx,qy,qz,qw,timeoffset**  
Unityと同じ左手系、かつ、ドライバー空間(ルーム空間変換なし)で仮想トラッカーを操作します。  
  
**/TrackerPoseRawDriver index,enable,x,y,z,qx,qy,qz,qw,timeoffset**  
OpenVRの右手系、かつ、ドライバー空間(ルーム空間変換なし)で仮想トラッカーを操作します。  
  
### ドライバ操作
**/Reset**  
すべてのトラッカーを非トラッキング状態にします。  
  
**/LoadSetting**  
ドライバーのjson設定を再読込します。  
  
**/SetRoomMatrix m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12**  
RoomToDriver空間変換行列を設定します。  
設定と同時にjsonに書き込むため、毎フレーム送るなど頻繁な送信は禁止します。  
