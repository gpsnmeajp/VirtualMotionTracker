/*
MIT License

Copyright (c) 2020 gpsnmeajp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "TrackedDeviceServerDriver.h"

#pragma warning(push)
#pragma warning(disable: 26812 )
namespace VMTDriver {
    const VRBoneTransform_t BindHandParentSpaceBonesLeft[skeletonBoneCount] = {
        {{0.00000000f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00015979f,-0.00003192f,0.00062571f,1.00000000f},{0.00000000f,0.00000000f,1.00000000f,0.00000000f}},
        {{-0.01791380f,0.02917804f,0.02529832f,1.00000000f},{0.27638680f,0.54119440f,0.18202920f,0.77303580f}},
        {{0.04040613f,0.00000000f,0.00000000f,1.00000000f},{0.96917300f,0.00006098f,-0.00137185f,0.24637750f}},
        {{0.03251682f,0.00000000f,0.00000000f,1.00000000f},{0.98817320f,0.00009757f,0.00140283f,0.15333510f}},
        {{0.03046400f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00155725f,0.02107323f,0.01478697f,1.00000000f},{0.55075310f,0.53105640f,-0.35143420f,0.53957770f}},
        {{0.07379755f,0.00000000f,0.00000000f,1.00000000f},{0.96898110f,0.00162347f,-0.05288696f,0.24140380f}},
        {{0.04328656f,0.00000000f,0.00000000f,1.00000000f},{0.98277030f,-0.00008593f,0.00504147f,0.18476210f}},
        {{0.02827520f,0.00000000f,0.00000000f,1.00000000f},{0.99707220f,0.00003136f,-0.00116592f,0.07645763f}},
        {{0.02282154f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00217731f,0.00711954f,0.01631884f,1.00000000f},{0.53342340f,0.56174980f,-0.41973730f,0.47298760f}},
        {{0.07088554f,0.00000000f,0.00000000f,1.00000000f},{0.97338810f,0.00000000f,-0.00018549f,0.22916260f}},
        {{0.04310854f,0.00000000f,0.00000000f,1.00000000f},{0.98752870f,0.00009328f,-0.00368821f,0.15739560f}},
        {{0.03326592f,0.00000000f,0.00000000f,1.00000000f},{0.98996160f,-0.00010740f,0.00412536f,0.14127660f}},
        {{0.02589229f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00051342f,-0.00654513f,0.01634766f,1.00000000f},{0.51669220f,0.55014350f,-0.49554790f,0.42988790f}},
        {{0.06597498f,0.00000000f,0.00000000f,1.00000000f},{0.97455590f,-0.00090130f,-0.04095693f,0.22036930f}},
        {{0.04033124f,0.00000000f,0.00000000f,1.00000000f},{0.99100110f,-0.00007314f,0.00253144f,0.13382990f}},
        {{0.02848879f,0.00000000f,0.00000000f,1.00000000f},{0.99078800f,0.00019866f,-0.00426307f,0.13535490f}},
        {{0.02243014f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00247816f,-0.01898137f,0.01521364f,1.00000000f},{-0.48575760f,-0.51532690f,0.61501610f,-0.34674640f}},
        {{0.06285565f,0.00000000f,0.00000000f,1.00000000f},{0.99349050f,0.00394219f,0.02816410f,0.11030770f}},
        {{0.02987433f,0.00000000f,0.00000000f,1.00000000f},{0.99111370f,0.00038264f,-0.01145603f,0.13252290f}},
        {{0.01797853f,0.00000000f,0.00000000f,1.00000000f},{0.99400630f,-0.00053848f,0.01270246f,0.10858110f}},
        {{0.01801792f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.03928087f,0.06008123f,-0.08449184f,1.00000000f},{0.04503725f,0.81958880f,-0.04861424f,-0.56910670f}},
        {{0.01823282f,0.03727836f,-0.14895560f,1.00000000f},{0.59722810f,0.70841940f,0.20955540f,-0.31232550f}},
        {{0.01256070f,0.00787113f,-0.15469030f,1.00000000f},{0.64706130f,0.67740290f,0.22114060f,-0.27117140f}},
        {{0.01786924f,-0.02323779f,-0.14223540f,1.00000000f},{0.72162900f,0.59502730f,0.23741430f,-0.26235200f}},
        {{0.01600805f,-0.04565187f,-0.11928000f,1.00000000f},{0.73903050f,0.51142090f,0.34899970f,-0.26548390f}}
    };
    const VRBoneTransform_t BindHandParentSpaceBonesRight[skeletonBoneCount] = {
        {{0.00000000f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00015979f,-0.00003192f,0.00062571f,1.00000000f},{0.00000000f,0.00000000f,1.00000000f,0.00000000f}},
        {{0.01791379f,0.02917817f,0.02529840f,1.00000000f},{0.54119440f,-0.27638680f,0.77303580f,-0.18202920f}},
        {{-0.04040596f,-0.00000005f,0.00000005f,1.00000000f},{0.96917300f,0.00006098f,-0.00137185f,0.24637750f}},
        {{-0.03251679f,-0.00000005f,-0.00000001f,1.00000000f},{0.98817320f,0.00009757f,0.00140283f,0.15333510f}},
        {{-0.03046390f,0.00000016f,0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00155722f,0.02107324f,0.01478686f,1.00000000f},{0.53105640f,-0.55075310f,0.53957770f,0.35143420f}},
        {{-0.07379761f,-0.00000002f,-0.00000009f,1.00000000f},{0.96898110f,0.00162347f,-0.05288696f,0.24140380f}},
        {{-0.04328668f,0.00000004f,0.00000015f,1.00000000f},{0.98277030f,-0.00008593f,0.00504147f,0.18476210f}},
        {{-0.02827519f,-0.00000003f,-0.00000013f,1.00000000f},{0.99707220f,0.00003136f,-0.00116592f,0.07645763f}},
        {{-0.02282138f,-0.00000014f,0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00217731f,0.00711954f,0.01631874f,1.00000000f},{0.56174980f,-0.53342340f,0.47298760f,0.41973730f}},
        {{-0.07088561f,0.00000011f,0.00000000f,1.00000000f},{0.97338810f,0.00000000f,-0.00018549f,0.22916260f}},
        {{-0.04310849f,-0.00000013f,0.00000002f,1.00000000f},{0.98752870f,0.00009328f,-0.00368821f,0.15739560f}},
        {{-0.03326598f,0.00000003f,-0.00000002f,1.00000000f},{0.98996160f,-0.00010740f,0.00412536f,0.14127660f}},
        {{-0.02589237f,0.00000010f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00051344f,-0.00654512f,0.01634769f,1.00000000f},{0.55014350f,-0.51669220f,0.42988790f,0.49554790f}},
        {{-0.06597489f,-0.00000001f,0.00000000f,1.00000000f},{0.97455590f,-0.00090130f,-0.04095693f,0.22036930f}},
        {{-0.04033120f,0.00000000f,0.00000000f,1.00000000f},{0.99100110f,-0.00007314f,0.00253144f,0.13382990f}},
        {{-0.02848878f,0.00000000f,0.00000007f,1.00000000f},{0.99078800f,0.00019866f,-0.00426307f,0.13535490f}},
        {{-0.02243024f,0.00000011f,-0.00000002f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00247815f,-0.01898137f,0.01521358f,1.00000000f},{0.51532690f,-0.48575760f,0.34674640f,0.61501610f}},
        {{-0.06285566f,0.00000002f,-0.00000002f,1.00000000f},{0.99349050f,0.00394219f,0.02816410f,0.11030770f}},
        {{-0.02987425f,-0.00000004f,0.00000009f,1.00000000f},{0.99111370f,0.00038264f,-0.01145603f,0.13252290f}},
        {{-0.01797869f,0.00000010f,-0.00000009f,1.00000000f},{0.99400630f,-0.00053848f,0.01270246f,0.10858110f}},
        {{-0.01801794f,-0.00000002f,0.00000007f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.03928085f,0.06008116f,-0.08449180f,1.00000000f},{0.81958890f,-0.04503726f,-0.56910660f,0.04861432f}},
        {{-0.01823281f,0.03727831f,-0.14895570f,1.00000000f},{0.70841930f,-0.59722800f,-0.31232550f,-0.20955540f}},
        {{-0.01256069f,0.00787113f,-0.15469020f,1.00000000f},{0.67740280f,-0.64706130f,-0.27117140f,-0.22114080f}},
        {{-0.01786923f,-0.02323769f,-0.14223530f,1.00000000f},{0.59502730f,-0.72162890f,-0.26235200f,-0.23741450f}},
        {{-0.01600804f,-0.04565192f,-0.11928010f,1.00000000f},{0.51142100f,-0.73903040f,-0.26548390f,-0.34899990f}}
    };
    const VRBoneTransform_t OpenHandParentSpaceBonesLeft[skeletonBoneCount] = {
        {{0.00000000f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00015979f,-0.00003192f,0.00062571f,1.00000000f},{0.00000000f,0.00000000f,1.00000000f,0.00000000f}},
        {{-0.01208323f,0.02807025f,0.02504969f,1.00000000f},{0.46411170f,0.56741810f,0.27210630f,0.62337400f}},
        {{0.04040596f,0.00000005f,-0.00000005f,1.00000000f},{0.99483840f,0.08293856f,0.01945437f,0.05512988f}},
        {{0.03251679f,0.00000005f,0.00000001f,1.00000000f},{0.97479270f,-0.00321332f,0.02186683f,-0.22201490f}},
        {{0.03046390f,-0.00000016f,-0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00063245f,0.02686615f,0.01500195f,1.00000000f},{0.64425150f,0.42197870f,-0.47820250f,0.42213310f}},
        {{0.07420439f,-0.00500220f,0.00023377f,1.00000000f},{0.99533170f,0.00700684f,-0.03912375f,0.08794935f}},
        {{0.04393007f,-0.00000006f,-0.00000018f,1.00000000f},{0.99789090f,0.04580838f,0.00214225f,-0.04594310f}},
        {{0.02869547f,0.00000009f,0.00000013f,1.00000000f},{0.99964880f,0.00185046f,-0.02278250f,-0.01340946f}},
        {{0.02282138f,0.00000014f,-0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00217731f,0.00711954f,0.01631874f,1.00000000f},{0.54672300f,0.54127650f,-0.44252020f,0.46074900f}},
        {{0.07095288f,0.00077883f,0.00099719f,1.00000000f},{0.98029450f,-0.16726140f,-0.07895869f,0.06936778f}},
        {{0.04310849f,0.00000010f,0.00000001f,1.00000000f},{0.99794670f,0.01849256f,0.01319235f,0.05988611f}},
        {{0.03326605f,0.00000001f,0.00000002f,1.00000000f},{0.99739390f,-0.00332781f,-0.02822515f,-0.06631514f}},
        {{0.02589237f,-0.00000010f,0.00000000f,1.00000000f},{0.99919460f,0.00000000f,0.00000000f,0.04012563f}},
        {{0.00051344f,-0.00654512f,0.01634769f,1.00000000f},{0.51669220f,0.55014350f,-0.49554790f,0.42988790f}},
        {{0.06587581f,0.00178579f,0.00069344f,1.00000000f},{0.99042010f,-0.05869612f,-0.10181950f,0.07249536f}},
        {{0.04069671f,0.00000010f,0.00000002f,1.00000000f},{0.99954500f,-0.00223973f,0.00000393f,0.03008105f}},
        {{0.02874696f,-0.00000010f,-0.00000005f,1.00000000f},{0.99910190f,-0.00072132f,-0.01269266f,0.04042039f}},
        {{0.02243024f,-0.00000011f,0.00000002f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00247815f,-0.01898137f,0.01521358f,1.00000000f},{0.52691830f,0.52394000f,-0.58402460f,0.32674030f}},
        {{0.06287840f,0.00284410f,0.00033151f,1.00000000f},{0.98660920f,-0.05961486f,-0.13516300f,0.06913206f}},
        {{0.03021971f,0.00000003f,0.00000009f,1.00000000f},{0.99431660f,0.00189612f,-0.00013151f,0.10644620f}},
        {{0.01818660f,0.00000001f,0.00000021f,1.00000000f},{0.99593060f,-0.00201019f,-0.05207913f,-0.07352567f}},
        {{0.01801794f,0.00000002f,-0.00000007f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00605911f,0.05628522f,0.06006384f,1.00000000f},{0.73723850f,0.20274540f,0.59426670f,0.24944110f}},
        {{-0.04041555f,-0.04301767f,0.01934458f,1.00000000f},{-0.29033050f,0.62352740f,-0.66380860f,-0.29373440f}},
        {{-0.03935372f,-0.07567404f,0.04704833f,1.00000000f},{-0.18704710f,0.67806250f,-0.65928520f,-0.26568340f}},
        {{-0.03834014f,-0.09098663f,0.08257892f,1.00000000f},{-0.18303720f,0.73679260f,-0.63475700f,-0.14393570f}},
        {{-0.03180600f,-0.08721431f,0.12101540f,1.00000000f},{-0.00365942f,0.75840720f,-0.63934180f,-0.12667810f}}
	};
	const VRBoneTransform_t OpenHandParentSpaceBonesRight[skeletonBoneCount] = {
        {{0.00000000f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00015979f,-0.00003192f,0.00062571f,1.00000000f},{0.00000000f,0.00000000f,1.00000000f,0.00000000f}},
        {{0.01208323f,0.02807025f,0.02504969f,1.00000000f},{0.56741810f,-0.46411170f,0.62337400f,-0.27210630f}},
        {{-0.04040596f,-0.00000005f,0.00000005f,1.00000000f},{0.99483840f,0.08293856f,0.01945437f,0.05512988f}},
        {{-0.03251679f,-0.00000005f,-0.00000001f,1.00000000f},{0.97479270f,-0.00321332f,0.02186683f,-0.22201490f}},
        {{-0.03046390f,0.00000016f,0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00063245f,0.02686615f,0.01500195f,1.00000000f},{0.42197860f,-0.64425150f,0.42213310f,0.47820250f}},
        {{-0.07420439f,0.00500220f,-0.00023377f,1.00000000f},{0.99533170f,0.00700684f,-0.03912375f,0.08794935f}},
        {{-0.04393007f,0.00000006f,0.00000018f,1.00000000f},{0.99789090f,0.04580838f,0.00214225f,-0.04594310f}},
        {{-0.02869547f,-0.00000009f,-0.00000013f,1.00000000f},{0.99964880f,0.00185046f,-0.02278250f,-0.01340946f}},
        {{-0.02282138f,-0.00000014f,0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00217731f,0.00711954f,0.01631874f,1.00000000f},{0.54127650f,-0.54672300f,0.46074910f,0.44252020f}},
        {{-0.07095288f,-0.00077883f,-0.00099719f,1.00000000f},{0.98029450f,-0.16726140f,-0.07895869f,0.06936778f}},
        {{-0.04310849f,-0.00000010f,-0.00000001f,1.00000000f},{0.99794670f,0.01849256f,0.01319235f,0.05988611f}},
        {{-0.03326605f,-0.00000001f,-0.00000002f,1.00000000f},{0.99739390f,-0.00332781f,-0.02822515f,-0.06631514f}},
        {{-0.02589237f,0.00000010f,0.00000000f,1.00000000f},{0.99919460f,0.00000000f,0.00000000f,0.04012563f}},
        {{-0.00051344f,-0.00654512f,0.01634769f,1.00000000f},{0.55014350f,-0.51669220f,0.42988790f,0.49554790f}},
        {{-0.06587581f,-0.00178579f,-0.00069344f,1.00000000f},{0.99042010f,-0.05869612f,-0.10181950f,0.07249536f}},
        {{-0.04069671f,-0.00000010f,-0.00000002f,1.00000000f},{0.99954500f,-0.00223973f,0.00000393f,0.03008105f}},
        {{-0.02874696f,0.00000010f,0.00000005f,1.00000000f},{0.99910190f,-0.00072132f,-0.01269266f,0.04042039f}},
        {{-0.02243024f,0.00000011f,-0.00000002f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00247815f,-0.01898137f,0.01521358f,1.00000000f},{0.52394000f,-0.52691830f,0.32674040f,0.58402460f}},
        {{-0.06287840f,-0.00284410f,-0.00033151f,1.00000000f},{0.98660920f,-0.05961486f,-0.13516300f,0.06913206f}},
        {{-0.03021971f,-0.00000003f,-0.00000009f,1.00000000f},{0.99431660f,0.00189612f,-0.00013151f,0.10644620f}},
        {{-0.01818660f,-0.00000001f,-0.00000021f,1.00000000f},{0.99593060f,-0.00201019f,-0.05207913f,-0.07352567f}},
        {{-0.01801794f,-0.00000002f,0.00000007f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00605911f,0.05628522f,0.06006384f,1.00000000f},{0.73723850f,0.20274540f,-0.59426670f,-0.24944110f}},
        {{0.04041555f,-0.04301767f,0.01934458f,1.00000000f},{-0.29033050f,0.62352740f,0.66380860f,0.29373440f}},
        {{0.03935372f,-0.07567404f,0.04704833f,1.00000000f},{-0.18704710f,0.67806250f,0.65928520f,0.26568340f}},
        {{0.03834014f,-0.09098663f,0.08257892f,1.00000000f},{-0.18303720f,0.73679260f,0.63475700f,0.14393570f}},
        {{0.03180600f,-0.08721431f,0.12101540f,1.00000000f},{-0.00365942f,0.75840720f,0.63934180f,0.12667810f}}
	};
	const VRBoneTransform_t FistParentSpaceBonesLeft[skeletonBoneCount] = {
        {{0.00000000f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00015979f,-0.00003192f,0.00062571f,1.00000000f},{0.00000000f,0.00000000f,1.00000000f,0.00000000f}},
        {{-0.01630509f,0.02752873f,0.01779966f,1.00000000f},{0.22570350f,0.48333190f,0.12641340f,0.83634190f}},
        {{0.04040596f,0.00000005f,-0.00000005f,1.00000000f},{0.89433530f,-0.01330204f,-0.08290180f,0.43944820f}},
        {{0.03251679f,0.00000005f,0.00000001f,1.00000000f},{0.84242780f,0.00065499f,0.00124436f,0.53880730f}},
        {{0.03046390f,-0.00000016f,-0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00380215f,0.02151419f,0.01280337f,1.00000000f},{0.61731450f,0.39517450f,-0.51087420f,0.44918510f}},
        {{0.07420439f,-0.00500220f,0.00023377f,1.00000000f},{0.73729080f,-0.03200609f,-0.11501260f,0.66494360f}},
        {{0.04328668f,-0.00000006f,-0.00000018f,1.00000000f},{0.61138080f,0.00328709f,0.00382346f,0.79132050f}},
        {{0.02827519f,0.00000009f,0.00000013f,1.00000000f},{0.74538840f,-0.00068361f,-0.00094501f,0.66662940f}},
        {{0.02282138f,0.00000014f,-0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00578692f,0.00680641f,0.01653390f,1.00000000f},{0.51420280f,0.52231500f,-0.47834850f,0.48369960f}},
        {{0.07095288f,0.00077883f,0.00099719f,1.00000000f},{0.72365310f,-0.09790080f,0.04854586f,0.68145800f}},
        {{0.04310849f,0.00000010f,0.00000001f,1.00000000f},{0.63746360f,-0.00236617f,-0.00283070f,0.77047170f}},
        {{0.03326598f,0.00000002f,0.00000002f,1.00000000f},{0.65800810f,0.00261038f,0.00319626f,0.75299950f}},
        {{0.02589237f,-0.00000010f,0.00000000f,1.00000000f},{0.99919460f,0.00000000f,0.00000000f,0.04012563f}},
        {{0.00412304f,-0.00685826f,0.01656286f,1.00000000f},{0.48960900f,0.52337400f,-0.52064350f,0.46399670f}},
        {{0.06587581f,0.00178579f,0.00069344f,1.00000000f},{0.75997040f,-0.05560936f,0.01157120f,0.64747100f}},
        {{0.04033121f,0.00000009f,0.00000002f,1.00000000f},{0.66431470f,0.00159458f,0.00196745f,0.74744870f}},
        {{0.02848878f,-0.00000010f,-0.00000005f,1.00000000f},{0.62695750f,-0.00278444f,-0.00323421f,0.77904180f}},
        {{0.02243024f,-0.00000011f,0.00000002f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00113146f,-0.01929451f,0.01542875f,1.00000000f},{0.47976630f,0.47783280f,-0.63019820f,0.37993450f}},
        {{0.06287840f,0.00284410f,0.00033151f,1.00000000f},{0.82700090f,0.03428218f,0.00343963f,0.56114380f}},
        {{0.02987425f,0.00000003f,0.00000009f,1.00000000f},{0.70218460f,-0.00671554f,-0.00928916f,0.71190260f}},
        {{0.01797869f,0.00000000f,0.00000021f,1.00000000f},{0.67685250f,0.00795551f,0.00991727f,0.73600890f}},
        {{0.01801794f,0.00000002f,-0.00000007f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.01971631f,0.00280172f,0.09393694f,1.00000000f},{0.37728590f,-0.54083070f,0.15044620f,-0.73656190f}},
        {{0.00017072f,0.01647311f,0.09651526f,1.00000000f},{-0.00645624f,0.02274707f,-0.93292740f,-0.35928710f}},
        {{0.00044797f,0.00153641f,0.11654340f,1.00000000f},{-0.03935703f,0.10514250f,-0.92883310f,-0.35307950f}},
        {{0.00394865f,-0.01486879f,0.13060750f,1.00000000f},{-0.05507131f,0.06869533f,-0.94401620f,-0.31793320f}},
        {{0.00326260f,-0.03468513f,0.13992580f,1.00000000f},{0.01969014f,-0.10074140f,-0.95733130f,-0.27014860f}}
	};
	const VRBoneTransform_t FistParentSpaceBonesRight[skeletonBoneCount] = {
        {{0.00000000f,0.00000000f,0.00000000f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{0.00015979f,-0.00003192f,0.00062571f,1.00000000f},{0.00000000f,0.00000000f,1.00000000f,0.00000000f}},
        {{0.01630509f,0.02752873f,0.01779966f,1.00000000f},{0.48333190f,-0.22570350f,0.83634190f,-0.12641340f}},
        {{-0.04040596f,-0.00000005f,0.00000005f,1.00000000f},{0.89433530f,-0.01330204f,-0.08290180f,0.43944820f}},
        {{-0.03251679f,-0.00000005f,-0.00000001f,1.00000000f},{0.84242780f,0.00065499f,0.00124436f,0.53880730f}},
        {{-0.03046390f,0.00000016f,0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00380215f,0.02151419f,0.01280337f,1.00000000f},{0.39517450f,-0.61731450f,0.44918520f,0.51087420f}},
        {{-0.07420439f,0.00500220f,-0.00023377f,1.00000000f},{0.73729080f,-0.03200609f,-0.11501260f,0.66494360f}},
        {{-0.04328668f,0.00000006f,0.00000018f,1.00000000f},{0.61138080f,0.00328709f,0.00382346f,0.79132050f}},
        {{-0.02827519f,-0.00000009f,-0.00000013f,1.00000000f},{0.74538840f,-0.00068361f,-0.00094501f,0.66662940f}},
        {{-0.02282138f,-0.00000014f,0.00000008f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00578692f,0.00680641f,0.01653390f,1.00000000f},{0.52231500f,-0.51420280f,0.48369960f,0.47834840f}},
        {{-0.07095288f,-0.00077883f,-0.00099719f,1.00000000f},{0.72365310f,-0.09790080f,0.04854586f,0.68145800f}},
        {{-0.04310849f,-0.00000010f,-0.00000001f,1.00000000f},{0.63746360f,-0.00236617f,-0.00283070f,0.77047170f}},
        {{-0.03326598f,-0.00000002f,-0.00000002f,1.00000000f},{0.65800810f,0.00261038f,0.00319626f,0.75299950f}},
        {{-0.02589237f,0.00000010f,0.00000000f,1.00000000f},{0.99919460f,0.00000000f,0.00000000f,0.04012563f}},
        {{-0.00412304f,-0.00685826f,0.01656286f,1.00000000f},{0.52337400f,-0.48960900f,0.46399680f,0.52064350f}},
        {{-0.06587581f,-0.00178579f,-0.00069344f,1.00000000f},{0.75997040f,-0.05560936f,0.01157120f,0.64747100f}},
        {{-0.04033121f,-0.00000009f,-0.00000002f,1.00000000f},{0.66431470f,0.00159458f,0.00196745f,0.74744870f}},
        {{-0.02848878f,0.00000010f,0.00000005f,1.00000000f},{0.62695750f,-0.00278444f,-0.00323421f,0.77904180f}},
        {{-0.02243024f,0.00000011f,-0.00000002f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.00113146f,-0.01929451f,0.01542875f,1.00000000f},{0.47783280f,-0.47976640f,0.37993450f,0.63019820f}},
        {{-0.06287840f,-0.00284410f,-0.00033151f,1.00000000f},{0.82700090f,0.03428218f,0.00343963f,0.56114380f}},
        {{-0.02987425f,-0.00000003f,-0.00000009f,1.00000000f},{0.70218460f,-0.00671554f,-0.00928916f,0.71190260f}},
        {{-0.01797869f,0.00000000f,-0.00000021f,1.00000000f},{0.67685250f,0.00795551f,0.00991727f,0.73600890f}},
        {{-0.01801794f,-0.00000002f,0.00000007f,1.00000000f},{1.00000000f,0.00000000f,0.00000000f,0.00000000f}},
        {{-0.01971631f,0.00280172f,0.09393694f,1.00000000f},{0.37728590f,-0.54083070f,-0.15044620f,0.73656190f}},
        {{-0.00017072f,0.01647311f,0.09651526f,1.00000000f},{-0.00645624f,0.02274707f,0.93292740f,0.35928710f}},
        {{-0.00044797f,0.00153641f,0.11654340f,1.00000000f},{-0.03935703f,0.10514250f,0.92883310f,0.35307950f}},
        {{-0.00394865f,-0.01486879f,0.13060750f,1.00000000f},{-0.05507131f,0.06869533f,0.94401620f,0.31793320f}},
        {{-0.00326260f,-0.03468513f,0.13992580f,1.00000000f},{0.01969014f,-0.10074140f,0.95733130f,0.27014860f}}
	};

    //** 内部向け関数群 **

    //自動更新を有効にするか
    bool TrackedDeviceServerDriver::s_autoUpdate = false;

    //仮想デバイスのコンストラクタ。(Listから暗黙的にコールされる)
    TrackedDeviceServerDriver::TrackedDeviceServerDriver()
    {
        m_deviceIndex = k_unTrackedDeviceIndexInvalid;
        m_propertyContainer = k_ulInvalidPropertyContainer;
    }

    //仮想デバイスのデストラクタ。(Listから暗黙的にコールされる)
    TrackedDeviceServerDriver::~TrackedDeviceServerDriver()
    {
    }

    //仮想デバイスにシリアル番号を設定
    void TrackedDeviceServerDriver::SetDeviceSerial(string serial)
    {
        m_serial = serial;
    }

    //仮想デバイスに内部Indexを設定
    void TrackedDeviceServerDriver::SetObjectIndex(uint32_t idx)
    {
        m_index = idx;
    }

    //仮想デバイスにOpenVR姿勢を設定
    void TrackedDeviceServerDriver::SetPose(DriverPose_t pose)
    {
        m_pose = pose;
    }

    //仮想デバイスに内部姿勢を設定
    void TrackedDeviceServerDriver::SetRawPose(RawPose rawPose)
    {
        m_poweron = true; //有効な姿勢なので電源オン状態にする

        if (s_autoUpdate) {
            //自動更新が有効なら内部姿勢を保存するのみ。(OpenVR姿勢は自動更新されるため)
            m_rawPose = rawPose;
        }
        else {
            //自動更新が無効ならば内部姿勢を保存し、OpenVR姿勢を更新する
            m_lastRawPose = m_rawPose; //差分を取るために前回値を取っておく
            m_rawPose = rawPose;
            SetPose(RawPoseToPose());
        }
    }

    //Joint計算を行う
    void TrackedDeviceServerDriver::CalcJoint(DriverPose_t& pose, string serial, ReferMode_t mode, Eigen::Affine3d& RoomToDriverAffin) {
        vr::TrackedDevicePose_t devicePoses[k_unMaxTrackedDeviceCount]{};

        //OpenVRから全トラッキングデバイスの情報を取得する
        VRServerDriverHost()->GetRawTrackedDevicePoses(0.0f, devicePoses, k_unMaxTrackedDeviceCount);

        //接続済みのデバイスの中から、シリアル番号でデバイスを検索する
        int index = SearchDevice(devicePoses, serial);

        //探索エラーが帰ってきたら
        if (index == k_unTrackedDeviceIndexInvalid) {
            //デバイス = 接続済み・無効
            RejectTracking(pose);
            return;
        }

        vr::TrackedDevicePose_t& devicePose = devicePoses[index];

        //参照元のトラッキングステータスを継承させる(Reject無効化時に意味あり)
        pose.poseIsValid = devicePose.bPoseIsValid;
        pose.result = devicePose.eTrackingResult;

        //デバイスのトラッキング状態が正常なら
        if (devicePose.bPoseIsValid) {
            //デバイスの変換行列を取得し、Eigenの行列に変換
            float* m = (float*)(devicePose.mDeviceToAbsoluteTracking.m);

            Eigen::Affine3d rootDeviceToAbsoluteTracking;
            rootDeviceToAbsoluteTracking.matrix() <<
                m[0 * 4 + 0], m[0 * 4 + 1], m[0 * 4 + 2], m[0 * 4 + 3],
                m[1 * 4 + 0], m[1 * 4 + 1], m[1 * 4 + 2], m[1 * 4 + 3],
                m[2 * 4 + 0], m[2 * 4 + 1], m[2 * 4 + 2], m[2 * 4 + 3],
                0.0, 0.0, 0.0, 1.0;

            //位置の座標系をデバイス基準にする
            Eigen::Translation3d pos(rootDeviceToAbsoluteTracking.translation());
            pose.vecWorldFromDriverTranslation[0] = pos.x();
            pose.vecWorldFromDriverTranslation[1] = pos.y();
            pose.vecWorldFromDriverTranslation[2] = pos.z();

            //回転の座標系をルーム基準にしたりデバイス基準にしたりする
            Eigen::Quaterniond rot;
            switch (m_rawPose.mode) {
            case ReferMode_t::Follow:
                rot = Eigen::Quaterniond(RoomToDriverAffin.rotation());
                break;
            case ReferMode_t::Joint:
            default:
                rot = Eigen::Quaterniond(rootDeviceToAbsoluteTracking.rotation());
                break;
            }

            pose.qWorldFromDriverRotation.x = rot.x();
            pose.qWorldFromDriverRotation.y = rot.y();
            pose.qWorldFromDriverRotation.z = rot.z();
            pose.qWorldFromDriverRotation.w = rot.w();

            //デバイス = 接続済み・有効・特殊座標系
            return;
        }
        else {
            //デバイス = 接続済み・無効
            RejectTracking(pose);
            return;
        }
    }

    //デバイスをシリアル番号から探す
    int TrackedDeviceServerDriver::SearchDevice(vr::TrackedDevicePose_t* poses, string serial)
    {
        IVRProperties* props = VRPropertiesRaw();
        CVRPropertyHelpers* helper = VRProperties();

        //デバイスシリアルが空白
        if (serial.empty()) {
            //探索エラーを返す
            return k_unTrackedDeviceIndexInvalid;
        }

        //デバイスシリアルがHMD(でかつ、HMD特別処理が有効なら)
        if (serial == "HMD" && Config::GetInstance()->GetHMDisIndex0()) {
            //HMDが接続OKなら
            if (poses[k_unTrackedDeviceIndex_Hmd].bDeviceIsConnected) {
                //HMDのインデックスを返す
                return k_unTrackedDeviceIndex_Hmd;
            }
            else {
                //(HMDがつながっていないのは普通ありえないが)探索エラーを返す
                return k_unTrackedDeviceIndexInvalid;
            }
        }

        //デバイスをOpenVR index順に調べる
        for (uint32_t i = 0; i < k_unMaxTrackedDeviceCount; i++) {
            //そのデバイスがつながっていないなら次のデバイスへ
            if (poses[i].bDeviceIsConnected != true) {
                continue;
            }

            //デバイスがつながっているので、シリアルナンバーを取得する
            PropertyContainerHandle_t h = props->TrackedDeviceToPropertyContainer(i);
            string SerialNumber = helper->GetStringProperty(h, ETrackedDeviceProperty::Prop_SerialNumber_String);

            //対象シリアルナンバーと比較し、違うデバイスなら、次のデバイスへ
            if (serial != SerialNumber) {
                continue;
            };

            //目的のデバイスを見つけたので返却
            return i;
        }
        //最後まで探したが、目的のデバイスは見つからなかった
        return k_unTrackedDeviceIndexInvalid;
    }

    //デバイスをトラッキング失敗状態にする
    void TrackedDeviceServerDriver::RejectTracking(DriverPose_t& pose)
    {
        //(ただし、設定から有効な場合のみ。そうでない場合は無視してトラッキングを継続する)
        if (Config::GetInstance()->GetRejectWhenCannotTracking()) {
            //デバイス = 接続済み・無効
            pose.poseIsValid = false;
            pose.result = TrackingResult_Running_OutOfRange;
        }
    }

    DriverPose_t TrackedDeviceServerDriver::RawPoseToPose()
    {
        DriverPose_t pose{ 0 };

        //ルーム変換行列の変換
        Eigen::Affine3d RoomToDriverAffin;
        RoomToDriverAffin = Config::GetInstance()->GetRoomToDriverMatrix();

        Eigen::Translation3d pos(RoomToDriverAffin.translation());
        Eigen::Quaterniond rot(RoomToDriverAffin.rotation());

        //OpenVR姿勢へ、一旦通常のデータを書き込む
        pose.poseTimeOffset = m_rawPose.timeoffset;

        pose.qWorldFromDriverRotation.x = rot.x();
        pose.qWorldFromDriverRotation.y = rot.y();
        pose.qWorldFromDriverRotation.z = rot.z();
        pose.qWorldFromDriverRotation.w = rot.w();

        pose.vecWorldFromDriverTranslation[0] = pos.x();
        pose.vecWorldFromDriverTranslation[1] = pos.y();
        pose.vecWorldFromDriverTranslation[2] = pos.z();

        pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;

        pose.vecDriverFromHeadTranslation[0] = 0.0f;
        pose.vecDriverFromHeadTranslation[1] = 0.0f;
        pose.vecDriverFromHeadTranslation[2] = 0.0f;

        pose.vecPosition[0] = m_rawPose.x;
        pose.vecPosition[1] = m_rawPose.y;
        pose.vecPosition[2] = m_rawPose.z;

        pose.vecVelocity[0] = 0.0f;
        pose.vecVelocity[1] = 0.0f;
        pose.vecVelocity[2] = 0.0f;

        pose.vecAcceleration[0] = 0.0f;
        pose.vecAcceleration[1] = 0.0f;
        pose.vecAcceleration[2] = 0.0f;

        pose.qRotation.x = m_rawPose.qx;
        pose.qRotation.y = m_rawPose.qy;
        pose.qRotation.z = m_rawPose.qz;
        pose.qRotation.w = m_rawPose.qw;

        pose.vecAngularVelocity[0] = 0.0f;
        pose.vecAngularVelocity[1] = 0.0f;
        pose.vecAngularVelocity[2] = 0.0f;

        pose.vecAngularAcceleration[0] = 0.0f;
        pose.vecAngularAcceleration[1] = 0.0f;
        pose.vecAngularAcceleration[2] = 0.0f;

        pose.result = TrackingResult_Running_OK;

        pose.poseIsValid = true;
        pose.willDriftInYaw = false;
        pose.shouldApplyHeadModel = false;

        pose.deviceIsConnected = true;

        //デバイスが有効でない場合、ステータスを無効で更新し、ここで返却
        if (m_rawPose.enable == 0) {
            pose.deviceIsConnected = false;
            pose.poseIsValid = false;
            pose.result = ETrackingResult::TrackingResult_Calibrating_OutOfRange;
            //デバイス = 非接続・無効
            return pose;
        }

        //ルームマトリクスが設定されていないとき、ステータスを無効で更新し、ここで返却
        if (!Config::GetInstance()->GetRoomMatrixStatus()) {
            //デバイス = 接続済み・無効
            RejectTracking(pose);
            return pose;
        }

        //トラッキングモードに合わせて処理する
        switch (m_rawPose.mode) {
            case ReferMode_t::None: {
                //通常のトラッキングモードの場合、何もしない
                //デバイス = 接続済み・有効・ルーム座標系

                //do noting
                break;
            }

            case ReferMode_t::Follow: {
                //デバイス = 接続済み・有効・デバイス位置座標系
                CalcJoint(pose, m_rawPose.root_sn, ReferMode_t::Follow, RoomToDriverAffin);
                break;
            }

            case ReferMode_t::Joint: {
                //デバイス = 接続済み・有効・デバイス位置回転座標系
                CalcJoint(pose, m_rawPose.root_sn, ReferMode_t::Joint, RoomToDriverAffin);
                break;
            }
            default: {
                //デバイス = 接続済み・無効
                RejectTracking(pose);
                break;
            }
        }
        return pose;
    }

    //仮想デバイスからOpenVRへデバイスの登録を依頼する
    //(ここではm_propertyContainerの操作はできない。この後にActivateがコールされる)
    void TrackedDeviceServerDriver::RegisterToVRSystem(int type)
    {
        if (!m_alreadyRegistered && !m_registrationInProgress)
        {
            switch (type)
            {
            case 1://Tracker
                LogInfo("RegisterToVRSystem: %s", "ETrackedDeviceClass::TrackedDeviceClass_GenericTracker");
                m_controllerRole = ControllerRole::None;
                m_deviceClass = ETrackedDeviceClass::TrackedDeviceClass_GenericTracker;
                break;
            case 2://Controller Left
                LogInfo("RegisterToVRSystem: %s", "ETrackedDeviceClass::TrackedDeviceClass_Controller (Left)");
                m_controllerRole = ControllerRole::Left;
                m_deviceClass = ETrackedDeviceClass::TrackedDeviceClass_Controller;
                break;
            case 3://Controller Right
                LogInfo("RegisterToVRSystem: %s", "ETrackedDeviceClass::TrackedDeviceClass_Controller (Right)");
                m_controllerRole = ControllerRole::Right;
                m_deviceClass = ETrackedDeviceClass::TrackedDeviceClass_Controller;
                break;
            case 4://TrackingReference
                LogInfo("RegisterToVRSystem: %s", "ETrackedDeviceClass::TrackedDeviceClass_TrackingReference");
                m_controllerRole = ControllerRole::None;
                m_deviceClass = ETrackedDeviceClass::TrackedDeviceClass_TrackingReference;
                break;
            default:
                LogError("RegisterToVRSystem: %s", "Unknown Type");
                m_deviceClass = ETrackedDeviceClass::TrackedDeviceClass_Invalid;
                return; //中止
            }
            m_registrationInProgress = true;
            LogIfFalse(VRServerDriverHost()->TrackedDeviceAdded(m_serial.c_str(), m_deviceClass, this));
        }
    }


    //仮想デバイスからOpenVRへデバイスのボタン状態の更新を通知する
    void TrackedDeviceServerDriver::UpdateButtonInput(uint32_t index, bool value, double timeoffset)
    {
        if (!m_alreadyRegistered) { return; }
        if (0 <= index && index < buttonCount)
        {
            LogIfEVRInputError(VRDriverInput()->UpdateBooleanComponent(ButtonComponent[index], value, timeoffset));
        }
        else {
            LogError("Index out of range: %u", index);
        }
    }

    //仮想デバイスからOpenVRへデバイスのトリガー(1軸)状態の更新を通知する
    void TrackedDeviceServerDriver::UpdateTriggerInput(uint32_t index, float value, double timeoffset)
    {
        if (!m_alreadyRegistered) { return; }
        if (value > 1.0) {
            value = 1.0;
        }
        if (value < 0) {
            value = 0;
        }
        if (isnan(value)) {
            value = 0;
        }

        if (0 <= index && index < triggerCount)
        {
            LogIfEVRInputError(VRDriverInput()->UpdateScalarComponent(TriggerComponent[index], value, timeoffset));
        }
        else {
            LogError("Index out of range: %u", index);
        }
    }

    //仮想デバイスからOpenVRへデバイスのジョイスティック(2軸)状態の更新を通知する
    void TrackedDeviceServerDriver::UpdateJoystickInput(uint32_t index, float x, float y, double timeoffset)
    {
        if (!m_alreadyRegistered) { return; }
        if (0 <= index && index < joystickCount/2)
        {
            LogIfEVRInputError(VRDriverInput()->UpdateScalarComponent(JoystickComponent[(index/2) + 0], x, timeoffset));
            LogIfEVRInputError(VRDriverInput()->UpdateScalarComponent(JoystickComponent[(index/2) + 1], y, timeoffset));
        }
        else {
            LogError("Index out of range: %u", index);
        }
    }

    //仮想デバイスからデバイスバッファへ個別の骨格状態を書き込む
    void TrackedDeviceServerDriver::WriteSkeletonInputBuffer(uint32_t index, VRBoneTransform_t bone)
    {
        if (!m_alreadyRegistered) { return; }
        if (m_controllerRole == ControllerRole::None) { return; } //コントローラでなければ受け付けない

        if (0 <= index && index < skeletonBoneCount)
        {
            m_boneTransform[index] = bone;
        }
        else {
            LogError("Index out of range: %u", index);
        }
    }

    //仮想デバイスからデバイスバッファへ指定Indexのボーンについて静的骨格をLerpした値を書き込む
    void TrackedDeviceServerDriver::WriteSkeletonInputBufferStaticLerpFinger(uint32_t finger, double t) {
        if (!m_alreadyRegistered) { return; }
        if (m_controllerRole == ControllerRole::None) { return; } //コントローラでなければ受け付けない

        switch ((SkeletonLerpFinder)finger) {
        case SkeletonLerpFinder::RootAndWrist:
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Root, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Wrist, t);
            break;
        case SkeletonLerpFinder::Thumb:
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Thumb0_ThumbProximal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Thumb1_ThumbIntermediate, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Thumb2_ThumbDistal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Thumb3_ThumbEnd, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Aux_Thumb_ThumbHelper, t);
            break;
        case SkeletonLerpFinder::Index:
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::IndexFinger0_IndexProximal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::IndexFinger1_IndexIntermediate, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::IndexFinger2_IndexDistal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::IndexFinger3_IndexDistal2, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::IndexFinger4_IndexEnd, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Aux_IndexFinger_IndexHelper, t);
            break;
        case SkeletonLerpFinder::Middle:
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::MiddleFinger0_MiddleProximal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::MiddleFinger1_MiddleIntermediate, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::MiddleFinger2_MiddleDistal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::MiddleFinger3_MiddleDistal2, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::MiddleFinger4_MiddleEnd, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Aux_MiddleFinger_MiddleHelper, t);
            break;
        case SkeletonLerpFinder::Ring:
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::RingFinger0_RingProximal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::RingFinger1_RingIntermediate, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::RingFinger2_RingDistal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::RingFinger3_RingDistal2, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::RingFinger4_RingEnd, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Aux_RingFinger_RingHelper, t);
            break;
        case SkeletonLerpFinder::PinkyLittle:
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::PinkyFinger0_LittleProximal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::PinkyFinger1_LittleIntermediate, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::PinkyFinger2_LittleDistal, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::PinkyFinger3_LittleDistal2, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::PinkyFinger4_LittleEnd, t);
            WriteSkeletonInputBufferStaticLerpBone((uint32_t)SkeletonBone::Aux_PinkyFinger_LittleHelper, t);
            break;
        default:LogError("Finger out of range: %u", finger); 
            break;
        }
    }


    //仮想デバイスからデバイスバッファへ指定Indexのボーンについて静的骨格をLerpした値を書き込む
    void TrackedDeviceServerDriver::WriteSkeletonInputBufferStaticLerpBone(uint32_t index, double t)
    {
        if (!m_alreadyRegistered) { return; }
        if (m_controllerRole == ControllerRole::None) { return; } //コントローラでなければ受け付けない

        if (0 <= index && index < skeletonBoneCount)
        {
            if (m_controllerRole == ControllerRole::Left) {
                Eigen::Quaterniond a(FistParentSpaceBonesLeft[index].orientation.w, FistParentSpaceBonesLeft[index].orientation.x, FistParentSpaceBonesLeft[index].orientation.y, FistParentSpaceBonesLeft[index].orientation.z);
                Eigen::Quaterniond b(OpenHandParentSpaceBonesLeft[index].orientation.w, OpenHandParentSpaceBonesLeft[index].orientation.x, OpenHandParentSpaceBonesLeft[index].orientation.y, OpenHandParentSpaceBonesLeft[index].orientation.z);
                Eigen::Quaterniond r = a.slerp(t, b);

                VRBoneTransform_t bone{};
                bone.position.v[0] = FistParentSpaceBonesLeft[index].position.v[0] * (1.0 - t) + OpenHandParentSpaceBonesLeft[index].position.v[0] * t;
                bone.position.v[1] = FistParentSpaceBonesLeft[index].position.v[1] * (1.0 - t) + OpenHandParentSpaceBonesLeft[index].position.v[1] * t;
                bone.position.v[2] = FistParentSpaceBonesLeft[index].position.v[2] * (1.0 - t) + OpenHandParentSpaceBonesLeft[index].position.v[2] * t;
                bone.orientation.x = r.x();
                bone.orientation.y = r.y();
                bone.orientation.z = r.z();
                bone.orientation.w = r.w();

                m_boneTransform[index] = bone;
            }
            else {
                Eigen::Quaterniond a(FistParentSpaceBonesRight[index].orientation.w, FistParentSpaceBonesRight[index].orientation.x, FistParentSpaceBonesRight[index].orientation.y, FistParentSpaceBonesRight[index].orientation.z);
                Eigen::Quaterniond b(OpenHandParentSpaceBonesRight[index].orientation.w, OpenHandParentSpaceBonesRight[index].orientation.x, OpenHandParentSpaceBonesRight[index].orientation.y, OpenHandParentSpaceBonesRight[index].orientation.z);
                Eigen::Quaterniond r = a.slerp(t, b);

                VRBoneTransform_t bone{};
                bone.position.v[0] = FistParentSpaceBonesRight[index].position.v[0] * (1.0 - t) + OpenHandParentSpaceBonesRight[index].position.v[0] * t;
                bone.position.v[1] = FistParentSpaceBonesRight[index].position.v[1] * (1.0 - t) + OpenHandParentSpaceBonesRight[index].position.v[1] * t;
                bone.position.v[2] = FistParentSpaceBonesRight[index].position.v[2] * (1.0 - t) + OpenHandParentSpaceBonesRight[index].position.v[2] * t;
                bone.orientation.x = r.x();
                bone.orientation.y = r.y();
                bone.orientation.z = r.z();
                bone.orientation.w = r.w();

                m_boneTransform[index] = bone;
            }
        }
        else {
            LogError("Index out of range: %u", index);
        }
    }

    //仮想デバイスからデバイスバッファへ静的骨格状態を書き込む
    void TrackedDeviceServerDriver::WriteSkeletonInputBufferStatic(SkeletonBonePoseStatic type)
    {
        if (!m_alreadyRegistered) { return; }
        if (m_controllerRole == ControllerRole::None) { return; } //コントローラでなければ受け付けない

        switch (type) {
        case SkeletonBonePoseStatic::BindHand:
            if (m_controllerRole == ControllerRole::Left) {
                for (int i = 0; i < skeletonBoneCount; i++) {
                    m_boneTransform[i] = BindHandParentSpaceBonesLeft[i];
                }
            }
            if (m_controllerRole == ControllerRole::Right) {
                for (int i = 0; i < skeletonBoneCount; i++) {
                    m_boneTransform[i] = BindHandParentSpaceBonesRight[i];
                }
            }
            break;
        case SkeletonBonePoseStatic::OpenHand:
            if (m_controllerRole == ControllerRole::Left) {
                for (int i = 0; i < skeletonBoneCount; i++) {
                    m_boneTransform[i] = OpenHandParentSpaceBonesLeft[i];
                }
            }
            if (m_controllerRole == ControllerRole::Right) {
                for (int i = 0; i < skeletonBoneCount; i++) {
                    m_boneTransform[i] = OpenHandParentSpaceBonesRight[i];
                }
            }
            break;
        case SkeletonBonePoseStatic::Fist:
            if (m_controllerRole == ControllerRole::Left) {
                for (int i = 0; i < skeletonBoneCount; i++) {
                    m_boneTransform[i] = FistParentSpaceBonesLeft[i];
                }
            }
            if (m_controllerRole == ControllerRole::Right) {
                for (int i = 0; i < skeletonBoneCount; i++) {
                    m_boneTransform[i] = FistParentSpaceBonesRight[i];
                }
            }
            break;
        default:
            LogError("Unkown type: %u", type);
            break;
        }
    }

    //仮想デバイスからOpenVRへデバイスバッファの骨格状態の更新を通知する
    void TrackedDeviceServerDriver::UpdateSkeletonInput(double timeoffset)
    {
        if (!m_alreadyRegistered) { return; }
        if (m_controllerRole == ControllerRole::None) { return;  } //コントローラでなければ受け付けない
        LogIfEVRInputError(VRDriverInput()->UpdateSkeletonComponent(SkeletonComponent, EVRSkeletalMotionRange::VRSkeletalMotionRange_WithController, m_boneTransform, skeletonBoneCount));
        LogIfEVRInputError(VRDriverInput()->UpdateSkeletonComponent(SkeletonComponent, EVRSkeletalMotionRange::VRSkeletalMotionRange_WithoutController, m_boneTransform, skeletonBoneCount));
    }

    //仮想デバイスの状態をリセットする
    void TrackedDeviceServerDriver::Reset()
    {
        m_poweron = false; //電源オフ状態にする

        if (!m_alreadyRegistered) { return; }
        DriverPose_t pose{ 0 };
        pose.qRotation = VMTDriver::HmdQuaternion_Identity;
        pose.qWorldFromDriverRotation = VMTDriver::HmdQuaternion_Identity;
        pose.qDriverFromHeadRotation = VMTDriver::HmdQuaternion_Identity;
        pose.deviceIsConnected = false;
        pose.poseIsValid = false;
        pose.result = ETrackingResult::TrackingResult_Calibrating_OutOfRange;
        SetPose(pose);

        //全状態を初期化する
        for (int i = 0; i < buttonCount; i++) {
            UpdateButtonInput(i, false, 0);
        }
        for (int i = 0; i < triggerCount; i++) {
            UpdateTriggerInput(i, 0, 0);
        }
        for (int i = 0; i < joystickCount / 2; i++) {
            UpdateJoystickInput(i, 0, 0, 0);
        }

        //骨格初期値を設定(コントローラのみ)
        WriteSkeletonInputBufferStatic(SkeletonBonePoseStatic::BindHand);
        UpdateSkeletonInput(0);
    }

    //デバッグコマンド処理(VMT Managerから送られてくる)
    std::string TrackedDeviceServerDriver::VMTDebugCommand(std::string command)
    {
        int index =0 ;
        float t = 0;
        sscanf(command.c_str(), "%d:%f", &index, &t);

        WriteSkeletonInputBufferStaticLerpFinger(index, t);
        /*
        if (command == "1") {
            WriteSkeletonInputBufferStatic(SkeletonBonePoseStatic::BindHand);
            UpdateSkeletonInput(0);
            return "BindHand OK";
        }
        if (command == "2") {
            WriteSkeletonInputBufferStatic(SkeletonBonePoseStatic::Fist);
            UpdateSkeletonInput(0);
            return "Fist OK";
        }
        if (command == "3") {
            WriteSkeletonInputBufferStatic(SkeletonBonePoseStatic::OpenHand);
            UpdateSkeletonInput(0);
            return "OpenHand OK";
        }
        return "? NG";
        */
        return std::to_string(index) + ":"+std::to_string(t);
    }

    //仮想デバイスからOpenVRへデバイスの姿勢の更新を通知する(サーバーから毎フレームコールされる)
    void TrackedDeviceServerDriver::UpdatePoseToVRSystem()
    {
        if (!m_alreadyRegistered) { return; }
        //姿勢を更新
        VRServerDriverHost()->TrackedDevicePoseUpdated(m_deviceIndex, GetPose(), sizeof(DriverPose_t));
    }

    //仮想デバイスでOpenVRイベントを処理する(サーバーからイベントがあるタイミングでコールされる)
    void TrackedDeviceServerDriver::ProcessEvent(VREvent_t& VREvent)
    {
        //未登録 or 電源オフ時は反応しない
        if (!m_alreadyRegistered || !m_poweron) {
            return;
        }

        //異常値を除去(なんで送られてくるんだ？)
        if (VREvent_VendorSpecific_Reserved_End < VREvent.eventType) {
            return;
        }

        switch (VREvent.eventType)
        {
        case EVREventType::VREvent_Input_HapticVibration:
            //バイブレーション
            if (VREvent.data.hapticVibration.componentHandle == HapticComponent) {
                OSCReceiver::SendHaptic(m_index, VREvent.data.hapticVibration.fFrequency, VREvent.data.hapticVibration.fAmplitude, VREvent.data.hapticVibration.fDurationSeconds);
            }
            break;
        default:
            //デバッグ用
            //if (m_serial == "VMT_0") {
            //    Log::printf("Event: %d\n",VREvent.eventType);
            //}
            break;
        }
    }

    //仮想デバイスの姿勢を、OpenVRに転送するたびに自動更新するか
    void TrackedDeviceServerDriver::SetAutoUpdate(bool enable)
    {
        s_autoUpdate = enable;
    }




    //** OpenVR向け関数群 **

    //OpenVRからのデバイス有効化コール
    EVRInitError TrackedDeviceServerDriver::Activate(uint32_t unObjectId)
    {
        LogInfo("Activate: %u", unObjectId);

        //OpenVR Indexの記録
        m_deviceIndex = unObjectId;

        //OpenVR プロパティコンテナの保持
        m_propertyContainer = VRProperties()->TrackedDeviceToPropertyContainer(unObjectId);

        //OpenVR デバイスプロパティの設定
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_TrackingSystemName_String, "VirtualMotionTracker"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_ModelNumber_String, m_serial.c_str()));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_SerialNumber_String, m_serial.c_str()));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_RenderModelName_String, "{vmt}vmt_rendermodel"));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_WillDriftInYaw_Bool, false));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_ManufacturerName_String, "VirtualMotionTracker"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_TrackingFirmwareVersion_String, Version.c_str()));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_HardwareRevision_String, Version.c_str()));

        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_ConnectedWirelessDongle_String, Version.c_str()));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DeviceIsWireless_Bool, true));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DeviceIsCharging_Bool, false));
        LogIfETrackedPropertyError(VRProperties()->SetFloatProperty(m_propertyContainer, Prop_DeviceBatteryPercentage_Float, 1.0f));

        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Firmware_UpdateAvailable_Bool, false));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Firmware_ManualUpdate_Bool, true));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_Firmware_ManualUpdateURL_String, "https://github.com/gpsnmeajp/VirtualMotionTracker"));
        LogIfETrackedPropertyError(VRProperties()->SetUint64Property(m_propertyContainer, Prop_HardwareRevision_Uint64, 0));
        LogIfETrackedPropertyError(VRProperties()->SetUint64Property(m_propertyContainer, Prop_FirmwareVersion_Uint64, 0));
        LogIfETrackedPropertyError(VRProperties()->SetUint64Property(m_propertyContainer, Prop_FPGAVersion_Uint64, 0));
        LogIfETrackedPropertyError(VRProperties()->SetUint64Property(m_propertyContainer, Prop_VRCVersion_Uint64, 0));
        LogIfETrackedPropertyError(VRProperties()->SetUint64Property(m_propertyContainer, Prop_RadioVersion_Uint64, 0));
        LogIfETrackedPropertyError(VRProperties()->SetUint64Property(m_propertyContainer, Prop_DongleVersion_Uint64, 0));



        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DeviceProvidesBatteryStatus_Bool, true));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DeviceCanPowerOff_Bool, true));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_Firmware_ProgrammingTarget_String, Version.c_str()));


        
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Firmware_ForceUpdateRequired_Bool, false));

        LogIfETrackedPropertyError(VRProperties()->SetUint64Property(m_propertyContainer, Prop_ParentDriver_Uint64, 0));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_ResourceRoot_String, "vmt"));
        std::string RegisteredDeviceType_String = std::string("vmt/");
        RegisteredDeviceType_String += m_serial.c_str();
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_RegisteredDeviceType_String, RegisteredDeviceType_String.c_str()));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_InputProfilePath_String, "{vmt}/input/vmt_profile.json")); //vmt_profile.jsonに影響する
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_NeverTracked_Bool, false));


        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Identifiable_Bool, true));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_CanWirelessIdentify_Bool, true));

        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_Firmware_RemindUpdate_Bool, false));

        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_ReportsTimeSinceVSync_Bool, false));

        LogIfETrackedPropertyError(VRProperties()->SetUint64Property(m_propertyContainer, Prop_CurrentUniverseId_Uint64, 2));
        LogIfETrackedPropertyError(VRProperties()->SetUint64Property(m_propertyContainer, Prop_PreviousUniverseId_Uint64, 2));

        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DisplaySupportsRuntimeFramerateChange_Bool, false));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_DisplaySupportsAnalogGain_Bool, false));
        LogIfETrackedPropertyError(VRProperties()->SetFloatProperty(m_propertyContainer, Prop_DisplayMinAnalogGain_Float, 1.0f));
        LogIfETrackedPropertyError(VRProperties()->SetFloatProperty(m_propertyContainer, Prop_DisplayMaxAnalogGain_Float, 1.0f));

        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceOff_String, "{vmt}/icons/Off32x32.png"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceSearching_String, "{vmt}/icons/Searching32x32.png"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{vmt}/icons/SearchingAlert32x32.png"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceReady_String, "{vmt}/icons/Ready32x32.png"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{vmt}/icons/ReadyAlert32x32.png"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{vmt}/icons/NotReady32x32.png"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceStandby_String, "{vmt}/icons/Standby32x32.png"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{vmt}/icons/StandbyAlert32x32.png"));
        LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{vmt}/icons/AlertLow32x32.png"));

        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasDisplayComponent_Bool, false));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasCameraComponent_Bool, false));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasDriverDirectModeComponent_Bool, false));
        LogIfETrackedPropertyError(VRProperties()->SetBoolProperty(m_propertyContainer, Prop_HasVirtualDisplayComponent_Bool, false));

        //LogIfETrackedPropertyError(VRProperties()->SetStringProperty(m_propertyContainer, vmt_profile.json, "NO_SETTING")); //設定不可
        LogIfETrackedPropertyError(VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerHandSelectionPriority_Int32, 0));

        //コントローラロール登録
        if (m_controllerRole == ControllerRole::Left) {
            LogInfo("Skeleton: %s", "Left");
            //コントローラロールヒントを設定
            LogIfETrackedPropertyError(VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_LeftHand));
            //指ボーン制限なし(既定の握りこぶしを使用)
            LogIfEVRInputError(VRDriverInput()->CreateSkeletonComponent(m_propertyContainer, "/input/skeleton/left", "/skeleton/hand/left", "/pose/raw", EVRSkeletalTrackingLevel::VRSkeletalTracking_Partial, nullptr, 0, &SkeletonComponent));
        }
        else if (m_controllerRole == ControllerRole::Right) {
            LogInfo("Skeleton: %s", "Right");
            //コントローラロールヒントを設定
            LogIfETrackedPropertyError(VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_RightHand));
            //指ボーン制限なし(既定の握りこぶしを使用)
            LogIfEVRInputError(VRDriverInput()->CreateSkeletonComponent(m_propertyContainer, "/input/skeleton/right", "/skeleton/hand/right", "/pose/raw", EVRSkeletalTrackingLevel::VRSkeletalTracking_Partial, nullptr, 0, &SkeletonComponent));
        }
        else {
            if (Config::GetInstance()->GetOptoutTrackingRole()) {
                LogIfETrackedPropertyError(VRProperties()->SetInt32Property(m_propertyContainer, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_OptOut)); //手に割り当てないように
                LogInfo("Optout: %s", "Yes");
            }
            else {
                LogInfo("Optout: %s", "No");
            }
        }

        //OpenVR デバイス入力情報の定義
        LogIfEVRInputError(VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button0/click", &ButtonComponent[0]));
        LogIfEVRInputError(VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button1/click", &ButtonComponent[1]));
        LogIfEVRInputError(VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button2/click", &ButtonComponent[2]));
        LogIfEVRInputError(VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button3/click", &ButtonComponent[3]));
        LogIfEVRInputError(VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button4/click", &ButtonComponent[4]));
        LogIfEVRInputError(VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button5/click", &ButtonComponent[5]));
        LogIfEVRInputError(VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button6/click", &ButtonComponent[6]));
        LogIfEVRInputError(VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/Button7/click", &ButtonComponent[7]));

        LogIfEVRInputError(VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/Trigger0/value", &TriggerComponent[0], EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedOneSided));
        LogIfEVRInputError(VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/Trigger1/value", &TriggerComponent[1], EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedOneSided));

        LogIfEVRInputError(VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/Joystick0/x", &JoystickComponent[0], EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided));
        LogIfEVRInputError(VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/Joystick0/y", &JoystickComponent[1], EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided));

        LogIfEVRInputError(VRDriverInput()->CreateHapticComponent(m_propertyContainer, "/output/haptic", &HapticComponent));

        m_alreadyRegistered = true;
        m_registrationInProgress = false;

        //骨格初期値を設定(コントローラのみ)
        WriteSkeletonInputBufferStatic(SkeletonBonePoseStatic::BindHand);

        return EVRInitError::VRInitError_None;
    }

    //OpenVRからのデバイス無効化コール
    void TrackedDeviceServerDriver::Deactivate()
    {
        LogMarker();
        m_deviceIndex = k_unTrackedDeviceIndexInvalid;
        m_propertyContainer = k_ulInvalidPropertyContainer;
    }

    //OpenVRからのデバイス電源オフコール
    void TrackedDeviceServerDriver::EnterStandby()
    {
        LogMarker();
        //電源オフ要求が来た
        Reset();
    }

    //OpenVRからのデバイス固有機能の取得(ない場合はnullptrを返す)
    void* TrackedDeviceServerDriver::GetComponent(const char* pchComponentNameAndVersion)
    {
        return nullptr;
    }

    //OpenVRからのデバイスのデバッグリクエスト
    void TrackedDeviceServerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
    {
        LogMarker();

        //デバッグ用
        //Log::printf("DebugRequest: %s", pchRequest);
        if (unResponseBufferSize > 0) {
            pchResponseBuffer[0] = '\0';
        }
    }

    //OpenVRからのデバイス姿勢取得
    float t = 0;
    DriverPose_t TrackedDeviceServerDriver::GetPose()
    {
        //自動更新が有効 AND デバイス登録済み AND 電源オン状態の場合
        if (s_autoUpdate && m_alreadyRegistered && m_poweron) {
            //加速度計算の自動更新を行う
            m_lastRawPose = m_rawPose;
            m_rawPose.time = std::chrono::system_clock::now();
            //姿勢情報の更新(他デバイス連動時に効果あり)
            SetPose(RawPoseToPose());
            //骨格情報の更新(これをしないと反映されないため)
            UpdateSkeletonInput(0);
        }

        //現在のOpenVR向け姿勢を返却する
        return m_pose;
    }
}
#pragma warning(pop)
