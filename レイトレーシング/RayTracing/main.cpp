#include<dxlib.h>
#include"Geometry.h"
#include <cmath>
const int screen_width = 640;
const int screen_height = 480;

unsigned int canvas[screen_height][screen_width];

//�q���g�ɂȂ�Ǝv���āA�F�X�Ɗ֐���p�ӂ��Ă���܂���
//�ʂɂ��̊֐����g��Ȃ���΂����Ȃ��킯�ł��A����ɉ���Ȃ���΂����Ȃ��킯�ł�
//����܂���B���C�g���[�V���O���ł��Ă���΍\���܂���B


// ���˃x�N�g�������߂�
// ���Ѓx�N�g�� InVec
// �@���x�N�g�� N
// ���˃x�N�g�� return �ŕԂ�
// R=I-2(I�EN)N�Ɋ�Â��Čv�Z����
// �������A�@���x�N�g���͐��K���ς݂Ƃ���
Vector3 ReflectVector(const Vector3& InVec,const Vector3& N)
{
	return InVec - N * 2 * (Dot(InVec, N));
}

float Clamp(float value, float minValue = 0.0f, float maxValue = 1.0f)
{
	return min(max(value, minValue), maxValue );
}

///���C(����)�Ƌ��̂̓����蔻��
///@param ray (���_����X�N���[���s�N�Z���ւ̃x�N�g��)
///@param plane ����
///@param t[out]��_�܂ł̋���
///@hint ���C�͐��K�����Ƃ����ق����g���₷�����낤
bool IsHitRayAndObject(const Position3& eye, const Vector3& ray, const Plane& plane, float& t) {
	// �����̕ύX�_(���̃R�����g�͏����Ȃ��悤��)

	//return Dot(ray, plane.N) < 0;

	auto k = Dot(-ray, plane.N); // 1�񂠂��藎�����
	auto h = Dot(eye, plane.N)-plane.offset; // ���_�ƕ��ʂ̍ŒZ����
	if (k <= 0.0f) {
		return false;
	}
	t = h / k;
	return true;
}

///���C(����)�Ƌ��̂̓����蔻��
///@param ray (���_����X�N���[���s�N�Z���ւ̃x�N�g��)
///@param sphere ��
///@param t[out]��_�܂ł̋���
///@hint ���C�͐��K�����Ƃ����ق����g���₷�����낤
bool IsHitRayAndObject(const Position3& eye,const Vector3& ray,const Sphere& sp, float& t) {
	//���C�����K���ς݂ł���O��Łc
	//
	//���_���狅�̒��S�ւ̃x�N�g��(����)�����܂�
	auto center = sp.pos - eye;

	//���S���王���ւ̓��ς��Ƃ�܂������x�N�g����
	auto d = Dot(center, ray);
	// 
	//�����x�N�g���ƃx�N�g�����������āA���S����̐����������_�����߂܂�
	auto rdash = ray * d;


	auto v = rdash - center;
	//auto vDist = v.Magnitude();//�����̑傫��

	//d�Ɣ��a�Ɛ����̑傫�������_�܂ł̋����������߂�
	// �Q�Ƃ��ɑ�����Ă�������

	auto vLen2 = Dot(v, v);//���g���Ȃ��Ȃ���Ƒ傫���̂Q��ɂȂ�
	auto radius2 = sp.radius * sp.radius;

	auto w = sqrtf(radius2 - vLen2);
	t = d - w;

	return vLen2 <= radius2;


}

// �F��rgb(�O�`�P�ɐ��K�����ꂽfloat)�Ńs�N�Z����ł�
// ��ʂ�X���W
// ��ʂ�Y���W
// ��(
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

///���C�g���[�V���O
///@param eye ���_���W
///@param sphere ���I�u�W�F�N�g(���̂��������ɂ���)
void RayTracing(const Position3& eye,const Sphere& sphere,const Plane& plane) {
	Vector3 lightVec = { 1.0f,-1.0f,-1.0f };
	lightVec.Normalize();
	//memset(canvas,0,sizeof(unsigned int) * screen_height * screen_width)
//#pragma omp parallel for
	for (int y = 0; y < screen_height; ++y) {//�X�N���[���c����
		
		for (int x = 0; x < screen_width; ++x) {//�X�N���[��������
			//�@���_�ƃX�N���[�����W���王���x�N�g�������
			Vector3 ray(x - screen_width /2 - eye.x, screen_height/2 - y- eye.y, -eye.z);
			ray.Normalize();
			//�A���K�����Ƃ�
			//�BIsHitRay�֐���True�������甒���h��Ԃ�
			float t = 0.0f;
			if (IsHitRayAndObject(eye, ray, sphere,t)) {
				// �f�B�t���[�Y�̎菇
				// �������Ăق������Ƃ́A����t(�n�_����̋���)
				// ��p���āA
				//�@ray*t�Ō�_�܂ł̎����x�N�g��R�����
				//�A��x��Ԃł����n�_���狅�̒��S�܂ł̃x�N�g��C�����
				//�BR����C���������ƂŁA���S�����_�܂ł̃x�N�g�������
				//�C�B�̃x�N�g���𐳋K�����邱�ƂŖ@���x�N�g��N�𓾂�
				//�D���C�g�x�N�g��lightVec�Ɠ��ς���邱�Ƃ�cos�Ƃ����߂�
				//�E����𖾂邳�𗘗p���āA�J���[�l�֏�Z����
				// �����������C�g�x�N�g���͔��]�����Ȃ���cos�����Ό���
				auto R = ray * t;
				auto C = sphere.pos - eye;
				auto N = (R - C).Normalized();
				float ambient = 0.1f; // ����ȏ�Â��Ȃ�Ȃ����邳
				float diffuse =Clamp(Dot(-lightVec,N))/*����*/;
				// �X�y�L�����̎菇
				// �@���˃x�N�g�������
				// �A���˃x�N�g���Ǝ����x�N�g��(�t)����ς���
				// �A�̌��ʂ�n��(10�悩20�悭�炢�H)����
				// �C���̌��ʂ��f�B�t���[�Y�ɂ���
				auto reflectVec = ReflectVector(lightVec, N);
				float power = 20.0f;
				//���ʔ��˂�s=(R�EV)^n
				float specular = pow(Clamp(Dot(-ray, reflectVec)), power);
				specular +=pow( 1.0f - Clamp(Dot(-ray, N)),1.3f);


				//���˃x�N�g���ƌ�_�̍��W����܂����C�g�����s��
				//���Ƃ̓����蔻����s��
				//�������ĂȂ���Ή������Ȃ��i���̂̐F�����̂܂܏o���j
				//���Ɠ��������珰�̐F�Ƌ��̂̃f�B�t���[�Y����Z
				//�@���˃x�N�g��R���v�Z
				//�A��_P���v�Z
				//�B�x�N�g��R�����ʂƌ������邩����
				//�C��������Ȃ炔���擾
				//���n�_�͌�_�Ȃ̂Œ���
				//�DP��R�Ƃ�����A����Ɍ�_P2�����߂�
				//�E�D�����Ƃɏ��ʂ̐F���擾
				//�F�E�̌��ʂƋ��̂̐F����Z
				auto R2 = ReflectVector(ray, N);
				auto P = eye + ray * t; 
				Color col2 = { 1,1,1 };
				Vector3 P2;
				if(IsHitRayAndObject(P, R2, plane, t))
				{
					P2 = P + R2 * t;
					
				}

				GetColorF2(GetFloorColor(P2), col2.x, col2.y, col2.z);

				//������F����ɂ́A����b�̌v�Z�����������
				//�@���x�N�g���y��

				// ��1.0,��0.5,��0.5
				Color col(1.0, 0.5, 0.5);
				DrawPixelWithFloat(x, y,
					Clamp(max(specular + col.x * col2.x * diffuse, ambient))/*��*/,
					Clamp(max(specular + col.y * col2.y * diffuse, ambient))/*��*/,
					Clamp(max(specular + col.z * col2.z * diffuse, ambient))/*��*/);
			}
			else if (IsHitRayAndObject(eye, ray, plane, t)) {
				// �����̕ύX�_(���̃R�����g�͏����Ȃ��悤��)
				auto pos = eye + ray * t;


				//������ꏊ���e�ɂȂ邩�ǂ����𔻒肷��
				//�����e�ishadow�j�ɓ�����Ȃ�A�F�𔼕��ɂ��Ă݂܂��傤
				//�@���̓��������ꏊpos+lightVec(�t)�ŋ��Ƃ̓����蔻����Ƃ�
				//�A


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
				//���h��Ԃ���DrawPixel�֐����g���B
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
	SetMainWindowText(_T("2016023_�����I��"));
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
		RayTracing(Position3(0, 0, 300),sp,plane); // Position3�̉�p��ς���ƌ`���ς��Ȃ��Ȃ�
		auto fps = GetFPS();
		DrawFormatString(10, 10, 0xffffff, L"fps:%lf", fps);
		ScreenFlip();
	}

	WaitKey();
	DxLib_End();
}