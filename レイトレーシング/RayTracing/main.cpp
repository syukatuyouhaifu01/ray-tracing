#include<dxlib.h>
#include"Geometry.h"
#include <cmath>
const int screen_width = 640;
const int screen_height = 480;

unsigned int canvas[screen_height][screen_width];

//ヒントになると思って、色々と関数を用意しておりますが
//別にこの関数を使わなければいけないわけでも、これに沿わなければいけないわけでも
//ありません。レイトレーシングができていれば構いません。


// 反射ベクトルを求める
// 入社ベクトル InVec
// 法線ベクトル N
// 反射ベクトル return で返す
// R=I-2(I・N)Nに基づいて計算する
// ただし、法線ベクトルは正規化済みとする
Vector3 ReflectVector(const Vector3& InVec,const Vector3& N)
{
	return InVec - N * 2 * (Dot(InVec, N));
}

float Clamp(float value, float minValue = 0.0f, float maxValue = 1.0f)
{
	return min(max(value, minValue), maxValue );
}

///レイ(光線)と球体の当たり判定
///@param ray (視点からスクリーンピクセルへのベクトル)
///@param plane 平面
///@param t[out]交点までの距離
///@hint レイは正規化しといたほうが使いやすいだろう
bool IsHitRayAndObject(const Position3& eye, const Vector3& ray, const Plane& plane, float& t) {
	// 今日の変更点(下のコメントは消さないように)

	//return Dot(ray, plane.N) < 0;

	auto k = Dot(-ray, plane.N); // 1回あたり落ちる量
	auto h = Dot(eye, plane.N)-plane.offset; // 視点と平面の最短距離
	if (k <= 0.0f) {
		return false;
	}
	t = h / k;
	return true;
}

///レイ(光線)と球体の当たり判定
///@param ray (視点からスクリーンピクセルへのベクトル)
///@param sphere 球
///@param t[out]交点までの距離
///@hint レイは正規化しといたほうが使いやすいだろう
bool IsHitRayAndObject(const Position3& eye,const Vector3& ray,const Sphere& sp, float& t) {
	//レイが正規化済みである前提で…
	//
	//視点から球体中心へのベクトル(視線)を作ります
	auto center = sp.pos - eye;

	//中心から視線への内積をとります＝＞ベクトル長
	auto d = Dot(center, ray);
	// 
	//視線ベクトルとベクトル長をかけて、中心からの垂線下した点を求めます
	auto rdash = ray * d;


	auto v = rdash - center;
	//auto vDist = v.Magnitude();//垂線の大きさ

	//dと半径と垂線の大きさから交点までの距離ｔを求めて
	// 参照ｔに代入してください

	auto vLen2 = Dot(v, v);//自身をない席すると大きさの２乗になる
	auto radius2 = sp.radius * sp.radius;

	auto w = sqrtf(radius2 - vLen2);
	t = d - w;

	return vLen2 <= radius2;


}

// 色をrgb(０～１に正規化されたfloat)でピクセルを打つ
// 画面のX座標
// 画面のY座標
// 赤(
// 
// 

void DrawPixelWithFloat(int x, int y, float r, float g, float b) {
	DrawPixel(x, y, GetColor(static_cast<int>(255 * r), static_cast <int> (255 * g), static_cast <int>(255 * b)));
}


using Color = Vector3;

unsigned int GetFloorColor(Vector3 pos)
{

	auto distance = 1 /*Clamp(t/1000.0f)*/;
	bool flg = (static_cast<int>(pos.x / 30) + static_cast<int>(pos.z / 30)) % 2;
	flg = pos.x * pos.z < 0 ? flg : !flg;
	if (flg)
	{
		return GetColor(255 * distance, 255 * distance, 255 * distance);
	}
	else
	{
		return GetColor(128 * distance, 128 * distance, 128 * distance);
	}
}

void GetColorF2(int color, float& r, float& g, float& b)
{
	int ir, ig, ib;
	GetColor2(color, &ir, &ig, &ib);
	r = (float)ir / 255;
	g = (float)ig / 255;
	b = (float)ib / 255;
}

///レイトレーシング
///@param eye 視点座標
///@param sphere 球オブジェクト(そのうち複数にする)
void RayTracing(const Position3& eye,const Sphere& sphere,const Plane& plane) {
	Vector3 lightVec = { 1.0f,-1.0f,-1.0f };
	lightVec.Normalize();
	//memset(canvas,0,sizeof(unsigned int) * screen_height * screen_width)
//#pragma omp parallel for
	for (int y = 0; y < screen_height; ++y) {//スクリーン縦方向
		
		for (int x = 0; x < screen_width; ++x) {//スクリーン横方向
			//①視点とスクリーン座標から視線ベクトルを作る
			Vector3 ray(x - screen_width /2 - eye.x, screen_height/2 - y- eye.y, -eye.z);
			ray.Normalize();
			//②正規化しとく
			//③IsHitRay関数がTrueだったら白く塗りつぶす
			float t = 0.0f;
			if (IsHitRayAndObject(eye, ray, sphere,t)) {
				// ディフューズの手順
				// 今回やってほしいことは、このt(始点からの距離)
				// を用いて、
				//①ray*tで交点までの視線ベクトルRを作る
				//②二度手間ですが始点から球体中心までのベクトルCを作る
				//③RからCを引くことで、中心から交点までのベクトルを作る
				//④③のベクトルを正規化することで法線ベクトルNを得る
				//⑤ライトベクトルlightVecと内積を取ることでcosθを求める
				//⑥これを明るさを利用して、カラー値へ乗算する
				// ※ただしライトベクトルは反転させないとcosが反対向く
				auto R = ray * t;
				auto C = sphere.pos - eye;
				auto N = (R - C).Normalized();
				float ambient = 0.1f; // これ以上暗くならない明るさ
				float diffuse =Clamp(Dot(-lightVec,N))/*内積*/;
				// スペキュラの手順
				// ①反射ベクトルを作る
				// ②反射ベクトルと視線ベクトル(逆)を内積する
				// ②の結果をn乗(10乗か20乗くらい？)する
				// ④この結果をディフューズにたす
				auto reflectVec = ReflectVector(lightVec, N);
				float power = 20.0f;
				//鏡面反射はs=(R・V)^n
				float specular = pow(Clamp(Dot(-ray, reflectVec)), power);
				specular +=pow( 1.0f - Clamp(Dot(-ray, N)),1.3f);


				//反射ベクトルと交点の座標からまたレイトレを行う
				//床との当たり判定を行う
				//あたってなければ何もしない（球体の色をそのまま出す）
				//床と当たったら床の色と球体のディフューズを乗算
				//①反射ベクトルRを計算
				//②交点Pを計算
				//③ベクトルRが床面と交差するか判定
				//④交差するならｔを取得
				//※始点は交点なので注意
				//⑤PとRとｔから、さらに交点P2を求める
				//⑥⑤をもとに床面の色を取得
				//⑦⑥の結果と球体の色を乗算
				auto R2 = ReflectVector(ray, N);
				auto P = eye + ray * t; 
				Color col2 = { 1,1,1 };
				Vector3 P2;
				if(IsHitRayAndObject(P, R2, plane, t))
				{
					P2 = P + R2 * t;
					
				}

				GetColorF2(GetFloorColor(P2), col2.x, col2.y, col2.z);

				//今から皆さんには、↑のbの計算式をきちんと
				//法線ベクトル及び

				// 赤1.0,緑0.5,青0.5
				Color col(1.0, 0.5, 0.5);
				DrawPixelWithFloat(x, y,
					Clamp(max(specular + col.x * col2.x * diffuse, ambient))/*赤*/,
					Clamp(max(specular + col.y * col2.y * diffuse, ambient))/*緑*/,
					Clamp(max(specular + col.z * col2.z * diffuse, ambient))/*青*/);
			}
			else if (IsHitRayAndObject(eye, ray, plane, t)) {
				// 今日の変更点(下のコメントは消さないように)
				auto pos = eye + ray * t;


				//今いる場所が影になるかどうかを判定する
				//もし影（shadow）に当たるなら、色を半分にしてみましょう
				//①この当たった場所pos+lightVec(逆)で球との当たり判定をとる
				//②


				if (IsHitRayAndObject(pos, -lightVec, sphere, t))
				{
					int ir, ig, ib;
					GetColor2(GetFloorColor(pos), &ir, &ig, &ib);
					DrawPixel(x, y, GetColor(ir * 0.4, ig * 0.4, ib * 0.4));

				}
				else
				{
					DrawPixel(x, y, GetFloorColor(pos));
				}

			}
			else {
				//※塗りつぶしはDrawPixel関数を使う。
				if (((x / 30) + (y / 30)) % 2 == 0) {
					DrawPixel(x, y, 0x6F66FF);
				}
			}
		}
	}
	//for (int y = 0; y < screen_height; ++y)
	//{
	//	for (int x = 0; x < screen_height; ++x)
	//	{
	//	}
	//}
}

int WINAPI WinMain(HINSTANCE , HINSTANCE, LPSTR , int) {
	ChangeWindowMode(true);
	SetGraphMode(screen_width, screen_height, 32);
	SetMainWindowText(_T("2016023_中原悠翔"));
	DxLib_Init();
	SetDrawScreen(DX_SCREEN_BACK);
	auto sp = Sphere(100, Position3(0, 0, -100));
	Plane plane = { Vector3(0,1,0),-100 };
	while (ProcessMessage() != -1)
	{
		ClearDrawScreen();
		char keystate[256];
		GetHitKeyStateAll(keystate);
		if (keystate[KEY_INPUT_LEFT])
		{
			sp.pos.x -= 8;
		}
		if (keystate[KEY_INPUT_RIGHT])
		{
			sp.pos.x += 8;
		}
		if (keystate[KEY_INPUT_UP])
		{
			sp.pos.y += 8;
		}
		if (keystate[KEY_INPUT_DOWN])
		{
			sp.pos.y -= 8;
		}
		if (keystate[KEY_INPUT_Z])
		{
			sp.pos.z -= 8;
		}
		if (keystate[KEY_INPUT_X])
		{
			sp.pos.z += 8;
		}
		RayTracing(Position3(0, 0, 300),sp,plane); // Position3の画角を変えると形が変わらなくなる
		auto fps = GetFPS();
		DrawFormatString(10, 10, 0xffffff, L"fps:%lf", fps);
		ScreenFlip();
	}

	WaitKey();
	DxLib_End();
}