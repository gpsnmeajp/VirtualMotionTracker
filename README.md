# VMT - Virtual Motion Tracker
The OSC Virtual OpenVR Tracker Driver for your DIY tracking devices.  
Very easy, simple, and robust.  

シンプルなOpenVR 仮想トラッカードライバです。  
OSCで姿勢(座標や回転)を受け取り、仮想的なトラッカーとしてSteamVR上で利用することができます。  
  
今まで難易度の高かったVRトラッキングデバイスの自作を、UnityでGameObjectにアタッチする程度の簡単な処理で行うことができるようになります。  
OpenVRのサンプルと仕様に沿った作りになっており、他のドライバのようにSteamVRのアップデートの度にクラッシュすることは起きにくい作りになっています。  

# [Download / ダウンロード](https://github.com/gpsnmeajp/VirtualMotionTracker/releases)  
# [Manual & sample(English)](docs/note_en.md)
# [説明書・サンプル(日本語)](docs/note.md)

**本ソフトウェアは、自分で問題を解決できる人向け(上級者向け)です。**  
**入れただけで何かができるツールではなく、他のアプリケーションやデバイスと組み合わせて使用します。**  
**This software for developers. (not for end user)**  
  
**64bit Windows 10環境でのみ動作します。**  
**Only works on 64bit Windows 10**  

**Steam VRがHMDを認識していない場合、利用できません。(HMD未接続、Oculus Link起動前など)**  
**It should not work when you are not connect HMD. (Ex. disconnected, not started Oculus Link)**  

VMT_005までにあった"起動しない"などのトラブルに関しては、VMT_006を使用するとエラーメッセージが表示されます。  
If you have trouble for VMT 005 such as not starting, use VMT 006 or after will shows error message.  

認識しないソフトがある場合、以下を確認してください。  
+ VMT_009以降を使用しているか
+ [SteamVRトラッカーロールの設定](docs/howto.md)
+ [SteamVRバインディングの割当](docs/binding.md)
+ C:\Program Files (x86)\Steam\config\steamvr.vrsettings からVMT周りの設定を削除してみる(必ずバックアップを取ってください)

Please check below if you meet software won't recognize VMT.  
+ Use after version of VMT_009
+ [SteamVR Tracker Role](docs/note_en.md)
+ [SteamVR binding (Pose)](docs/binding.md)
+ Try remove VMT setting on C:\Program Files (x86)\Steam\config\steamvr.vrsettings (You must backup before change it.)

# サポートは閉鎖しました。(Support desk has been closed.)
説明書を読んで理解できない場合は、使用されないことを勧めします。  
Please don't use if you can not understand manual.  

コードレベル以外のIssueは返答無くCloseすることがあります。  
Issues other than code level may be closed without a response.  

明らかな不具合や異常挙動に関しては、詳細を添えてIssueを立ててください。  

# 他のソフトウェアに同梱されたVMTを使っている人へ (To VMT users bundled with other software)
このソフトは、無料、無保証で、使い方がわかる開発者に向けて作成しています。  
各ソフトウェアとの組み合わせで発生したトラブルは、各作者にサポートの依頼をしてください。  
This software is free, non-warranty, and designed for developers who know how to use it.  
For troubles that occur in combination with each software, ask each author for support.  
  
<img src="https://github.com/gpsnmeajp/VirtualMotionTracker/blob/master/docs/vmt_vr.png?raw=true"></img>
<img src="https://github.com/gpsnmeajp/VirtualMotionTracker/blob/master/docs/screen0.png?raw=true"></img>
<img src="https://github.com/gpsnmeajp/VirtualMotionTracker/blob/master/docs/Architecture.png?raw=true"></img>

# Licence
MIT Licence

# Keyword
Virtual Motion Tracker  
バーチャルモーショントラッカー    
OpenVR 仮想トラッカー  
OSCトラッカー  
ばもとら  

# 苦労話
[Virtual Motion Trackerを作ったときのノウハウ](https://qiita.com/gpsnmeajp/items/9c41654e6c89c6b9702f)
