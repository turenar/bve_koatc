// �ȉ��� ifdef �u���b�N�� DLL ����ȒP�ɃG�N�X�|�[�g������}�N�����쐬����W���I�ȕ��@�ł��B 
// ���� DLL ���̂��ׂẴt�@�C���̓R�}���h���C���Œ�`���ꂽ ATS_EXPORTS �V���{��
// �ŃR���p�C������܂��B���̃V���{���͂��� DLL ���g�p����ǂ̃v���W�F�N�g��ł�����`�łȂ�
// ��΂Ȃ�܂���B���̕��@�ł̓\�[�X�t�@�C���ɂ��̃t�@�C�����܂ނ��ׂẴv���W�F�N�g�� DLL 
// ����C���|�[�g���ꂽ���̂Ƃ��� ATS_API �֐����Q�Ƃ��A���̂��߂��� DLL �͂��̃}�N 
// ���Œ�`���ꂽ�V���{�����G�N�X�|�[�g���ꂽ���̂Ƃ��ĎQ�Ƃ��܂��B
//#pragma data_seg(".shared")
//#pragma data_seg()

#pragma once
#include <list>
#include "atsplugin.h"
#include "koatc.h"
#include "Monitor.h"

extern TCHAR g_fullpath[_MAX_PATH];
extern TCHAR g_drive[_MAX_DRIVE];
extern TCHAR g_dir[_MAX_PATH];
extern TCHAR g_fname[_MAX_FNAME];
extern TCHAR g_ext[_MAX_EXT];
extern TCHAR g_path[_MAX_PATH];
extern TCHAR g_inifilepath[_MAX_PATH];

extern int g_EmrBrake; // ���m�b�`
extern int g_SvcBrake; // ��p�ő�m�b�`
extern bool g_pilotlamp; // �p�C���b�g�����v
extern int g_time; // ���ݎ���
extern float g_speed; // ���x�v�̑��x[km/h]
extern int g_deltaT; // �t���[������[ms/frame]
extern double g_location;	//���݈ʒu
extern int g_sta_dopen_timer; // �h�A�J����w��ԃp�^�[�������܂ł̃^�C�}�[
extern int g_sta_dopen_wait; // ��̎���
extern int g_genLog;
extern std::list<int> beaconIdx;
extern std::list<int> beaconSig;
extern std::list<float> beaconDist;
extern std::list<int> beaconOpt;

extern ATS_HANDLES g_output; // �o��

void BeaconProcessor(int, int, float, int);
void BeaconReader(void);
void PanelAssign(int*);
void SoundAssign(int*);

extern CKoAtc g_koatc;

extern CMon g_mon;
