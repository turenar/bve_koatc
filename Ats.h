// �ȉ��� ifdef �u���b�N�� DLL ����ȒP�ɃG�N�X�|�[�g������}�N�����쐬����W���I�ȕ��@�ł��B 
// ���� DLL ���̂��ׂẴt�@�C���̓R�}���h���C���Œ�`���ꂽ ATS_EXPORTS �V���{��
// �ŃR���p�C������܂��B���̃V���{���͂��� DLL ���g�p����ǂ̃v���W�F�N�g��ł�����`�łȂ�
// ��΂Ȃ�܂���B���̕��@�ł̓\�[�X�t�@�C���ɂ��̃t�@�C�����܂ނ��ׂẴv���W�F�N�g�� DLL 
// ����C���|�[�g���ꂽ���̂Ƃ��� ATS_API �֐����Q�Ƃ��A���̂��߂��� DLL �͂��̃}�N 
// ���Œ�`���ꂽ�V���{�����G�N�X�|�[�g���ꂽ���̂Ƃ��ĎQ�Ƃ��܂��B
//#pragma data_seg(".shared")
//#pragma data_seg()

TCHAR g_fullpath[_MAX_PATH];
TCHAR g_drive[_MAX_DRIVE];
TCHAR g_dir[_MAX_PATH];
TCHAR g_fname[_MAX_FNAME];
TCHAR g_ext[_MAX_EXT];
TCHAR g_path[_MAX_PATH];
TCHAR g_inifilepath[_MAX_PATH];

int g_EmrBrake; // ���m�b�`
int g_SvcBrake; // ��p�ő�m�b�`
bool g_pilotlamp; // �p�C���b�g�����v
int g_time; // ���ݎ���
float g_speed; // ���x�v�̑��x[km/h]
int g_deltaT; // �t���[������[ms/frame]
double g_location;	//���݈ʒu
int g_genLog;
std::list<int> beaconIdx;
std::list<int> beaconSig;
std::list<float> beaconDist;
std::list<int> beaconOpt;

ATS_HANDLES g_output; // �o��

void BeaconProcessor(int, int, float, int);
void BeaconReader(void);
void PanelAssign(int*);
void SoundAssign(int*);

CKoAtc g_koatc;

CMon g_mon;
