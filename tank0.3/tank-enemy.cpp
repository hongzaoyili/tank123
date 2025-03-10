#include "common.h"
#include "tank-enemy.h"
#include "tank-player.h"
#include "mci-sound.h"

extern TankEnemy tankEnemyArr[MAX_TANK_ENEMY];
extern int mCurEnemyTankNum; // 当前界面中出现的坦克数量
extern int mTotalOutEnemyTank; // 累计已经出现的敌机坦克
extern HDC center_hdc; // 中间游戏区域，分开绘制方便进行更新
extern char map26x26[26][26]; // 地图数据
extern TankPlayer tankPlayer0; // 定义玩家坦克结构

int add_enemy_counter = 0;
extern bool mCampDie;	// 大本营是否被击中
extern bool showCampBomb;      // 显示大本营被击中爆炸效果
extern int gameOverCounter;
extern int gameOverX, gameOverY;		// 图片左上角坐标

/**
	增加一个敌机坦克
*/
void tank_enemy_add() {
	if (add_enemy_counter++ % 100 != 0) { // 每隔100*15ms出现一辆坦克
		return;
	}
	add_enemy_counter = 0;
	
	if (mCurEnemyTankNum >= 6 || mTotalOutEnemyTank >= 20 || (mCurEnemyTankNum + mTotalOutEnemyTank) >= 20) { // 当前不需要增加坦克
		return;
	}

	TankEnemy* pTankEnemy = &tankEnemyArr[mTotalOutEnemyTank]; // 获取当前要操作的数组中坦克对象
	
	//{ BOX_SIZE, BOX_SIZE * 13, BOX_SIZE * 25 }
	pTankEnemy->bornPosX[0] = BOX_SIZE;
	pTankEnemy->bornPosX[1] = BOX_SIZE*13;
	pTankEnemy->bornPosX[2] = BOX_SIZE*25;
	
	// int mMoveSpeedDev[4] = { 30, 28, 27, 25 };
	pTankEnemy->mMoveSpeedDev[0] = 30;
	pTankEnemy->mMoveSpeedDev[1] = 28;
	pTankEnemy->mMoveSpeedDev[2] = 27;
	pTankEnemy->mMoveSpeedDev[3] = 25;



	pTankEnemy->mDied = false; // 未挂
	pTankEnemy->mBorned = true; // 已经出生
	pTankEnemy->mTankMoving = false;
	pTankEnemy->mTankImageDirIndex = 0;
	pTankEnemy->mTankX = pTankEnemy->bornPosX[rand() % 3];
	pTankEnemy->mTankY = BOX_SIZE;
	pTankEnemy->mMoveStep = rand() % 200; // 随机移动的定时器计数值

	// 设置敌机坦克级别
	if (mTotalOutEnemyTank < 17) {
		pTankEnemy->mTankLevel = rand() % 3; // 随机返回0、1、2级别坦克
	}	
	else {
		pTankEnemy->mTankLevel = 3;
	}

	// 加载坦克资源，现在只加载普通的灰色坦克，不加载道具坦克和其他颜色的重型坦克
	TCHAR c[100];
	for (int dir = 0; dir < 4; dir++) { // i表示随机级别坦克的四个方向
		_stprintf_s(c, L"./res/big/gray-tank/%d-%d-1.gif", pTankEnemy->mTankLevel + 1, dir + 1);
		loadimage(&pTankEnemy->mTankImage[dir][0], c);
		_stprintf_s(c, L"./res/big/gray-tank/%d-%d-2.gif", pTankEnemy->mTankLevel + 1, dir + 1);
		loadimage(&pTankEnemy->mTankImage[dir][1], c);
	}

	// 初始化四角星资源与状态
	TCHAR buf[100] = { 0 };
	// 出生四角星闪烁
	for (int i = 0; i < 4; i++)
	{
		_stprintf_s(buf, _T("./res/big/star%d.gif"), i);
		loadimage(&pTankEnemy->mStar.mStarImage[i], buf);
	}
	pTankEnemy->mStar.starState = Star_Begin; // 四角星刚出现
	pTankEnemy->mStar.mStarCounter = 0;
	pTankEnemy->mStar.mStarIndex = 0;
	pTankEnemy->mStar.mStarIndexDir = 1; // 初始下标是由小变大

	// 初始化坦克移动、子弹速度、爆炸速度计数器
	clock_init(&pTankEnemy->mTankMoveTimer, pTankEnemy->mMoveSpeedDev[pTankEnemy->mTankLevel]);
	clock_init(&pTankEnemy->mBulletTimer, TANK_ENEMY_BULLET_SPEED);
	clock_init(&pTankEnemy->mShootTimer, rand() % 1000 + 600); // 设置敌机坦克发射炮弹随机间隔时间ms，只设置一次，后面固定不变

	// 加载炮弹图片资源
	TCHAR bulletBuf[100];
	for (int i = 0; i < 4; i++)
	{
		_stprintf_s(bulletBuf, _T("./res/big/bullet-%d.gif"), i);
		loadimage(&pTankEnemy->mBullet.mImage[i], bulletBuf);
	}
	pTankEnemy->mBullet.needDraw = false;
	pTankEnemy->mBullet.speed[0] = 2;
	pTankEnemy->mBullet.speed[1] = 3;
	pTankEnemy->mBullet.speed[2] = 3;
	pTankEnemy->mBullet.speed[3] = 4;
	pTankEnemy->mBullet.bulletSize[0][0] = 4;
	pTankEnemy->mBullet.bulletSize[0][1] = 3;
	pTankEnemy->mBullet.bulletSize[1][0] = 3;
	pTankEnemy->mBullet.bulletSize[1][1] = 4;
	pTankEnemy->mBullet.bulletSize[2][0] = 4;
	pTankEnemy->mBullet.bulletSize[2][1] = 3;
	pTankEnemy->mBullet.bulletSize[3][0] = 3;
	pTankEnemy->mBullet.bulletSize[3][1] = 4;
	
	pTankEnemy->mBullet.bullet_bias[0][0] = -BOX_SIZE;
	pTankEnemy->mBullet.bullet_bias[0][1] = -1;
	pTankEnemy->mBullet.bullet_bias[1][0] = -2;
	pTankEnemy->mBullet.bullet_bias[1][1] = -BOX_SIZE;
	pTankEnemy->mBullet.bullet_bias[2][0] = BOX_SIZE;
	pTankEnemy->mBullet.bullet_bias[2][1] = -1;
	pTankEnemy->mBullet.bullet_bias[3][0] = -2;
	pTankEnemy->mBullet.bullet_bias[3][1] = BOX_SIZE;
		
	// 加载爆炸效果图片
	for (int i = 0; i < 3; i++)
	{
		_stprintf_s(buf, _T("./res/big/bumb%d.gif"), i + 1);
		loadimage(&pTankEnemy->mBombStruct.bombImage[i], buf);
	}
	pTankEnemy->mBombStruct.bombCounter = 0;
	pTankEnemy->mBombStruct.showBomb = false;

	// 加载敌机被击中的大爆炸效果
	pTankEnemy->mBlastStruct.showBlast = false;
	for (int i = 0; i < 5; i++)
	{
		_stprintf_s(buf, _T("./res/big/blast/%d.gif"), i);
		loadimage(&pTankEnemy->mBlastStruct.blastImage[i], buf);
	}
	pTankEnemy->mBlastStruct.blastCounter = 0;

	// 加载敌机爆炸之后显示的分数资源
	loadimage(&pTankEnemy->mScoreImage[0], _T("./res/big/100.gif"));
	loadimage(&pTankEnemy->mScoreImage[1], _T("./res/big/200.gif"));
	loadimage(&pTankEnemy->mScoreImage[2], _T("./res/big/300.gif"));
	loadimage(&pTankEnemy->mScoreImage[3], _T("./res/big/400.gif"));
	
	
	// int mMoveSpeed[4] = { 1, 1, 1, 1 };
	pTankEnemy->mMoveSpeed[0] = 1;
	pTankEnemy->mMoveSpeed[1] = 1;
	pTankEnemy->mMoveSpeed[2] = 1;
	pTankEnemy->mMoveSpeed[3] = 1;

	
	pTankEnemy->dir = ENE_DIR_DOWN;
	mCurEnemyTankNum++;
	mTotalOutEnemyTank++; // 最后将出现的坦克总数+1
}

/**
	显示敌机坦克四角星
*/
void tank_enemy_show_star() {
	for (int i = 0; i < mTotalOutEnemyTank; i++) {
		TankEnemy* pTankEnemy = &tankEnemyArr[i];
		if (pTankEnemy->mDied == false && pTankEnemy->mBorned == true) { // 只判断那些刚出生且未挂掉的坦克
			if (pTankEnemy->mStar.starState == Star_End) { // 如果坦克已经出现则不显示四角星
				continue;
			}

			if (pTankEnemy->mStar.mStarCounter % 2 == 0) { // 主循环如果是偶数次计数，则切四角星图片
				if (pTankEnemy->mStar.mStarIndex > 3) {
					pTankEnemy->mStar.mStarIndexDir = -1; // 开始由大变小
				}
				else if (pTankEnemy->mStar.mStarIndex < 0) {
					pTankEnemy->mStar.mStarIndexDir = 1; // 开始由小变大
				}

				pTankEnemy->mStar.mStarIndex += pTankEnemy->mStar.mStarIndexDir;
			}
			if (pTankEnemy->mStar.mStarCounter++ > 25) {
				pTankEnemy->mStar.starState = Star_End; // 四角星闪烁完成
				continue;
			}

			// 按照计算的下标进行四角星图片绘制
			TransparentBlt(center_hdc,
				pTankEnemy->mTankX - BOX_SIZE, pTankEnemy->mTankY - BOX_SIZE, BOX_SIZE * 2, BOX_SIZE * 2,
				GetImageHDC(&pTankEnemy->mStar.mStarImage[pTankEnemy->mStar.mStarIndex]),
				0, 0, BOX_SIZE * 2, BOX_SIZE * 2,
				0x000000);
		}
	}
}

/**
	绘制敌机坦克和炮弹
*/
void tank_enemy_draw_tank() {
	for (int i = 0; i < mTotalOutEnemyTank; i++) {
		TankEnemy* pTankEnemy = &tankEnemyArr[i];
		if (pTankEnemy->mDied == false && 
			pTankEnemy->mBorned == true &&
			pTankEnemy->mStar.starState == Star_End) { // 处于活着状态且已经出现过四角星的坦克
			
			pTankEnemy->mTankImageDirIndex = pTankEnemy->mTankImageDirIndex == 0 ? 1 : 0; // 切换坦克移动方向图片
			IMAGE tankImg = pTankEnemy->mTankImage[pTankEnemy->dir][pTankEnemy->mTankImageDirIndex];
			TransparentBlt(center_hdc, pTankEnemy->mTankX - BOX_SIZE, pTankEnemy->mTankY - BOX_SIZE,
				BOX_SIZE * 2, BOX_SIZE * 2,
				GetImageHDC(&tankImg),
				0, 0,
				BOX_SIZE * 2, BOX_SIZE * 2,
				0x000000);

			// 检测炮弹是否遇到障碍物，遇到障碍物会停止绘制炮弹并通知产生爆炸效果
			check_enemy_bullet_to_obstacle(pTankEnemy);
			
			// 是否需要显示炮弹的爆炸效果
			if (pTankEnemy->mBombStruct.showBomb) { // 需要显示爆炸效果
				int index[6] = { 0,1,1,2,2,1 };	// 图片下标切换顺序
				if (pTankEnemy->mBombStruct.bombCounter >= 6) {
					pTankEnemy->mBombStruct.bombCounter = 0;
					pTankEnemy->mBombStruct.showBomb = false;
					pTankEnemy->mBullet.needDraw = false; // 前一发炮弹爆炸之后，再开始响应玩家新的发射炮弹请求
				}
				else {
					TransparentBlt(center_hdc,
						pTankEnemy->mBombStruct.mBombX - BOX_SIZE, pTankEnemy->mBombStruct.mBombY - BOX_SIZE,
						BOX_SIZE * 2, BOX_SIZE * 2,
						GetImageHDC(&pTankEnemy->mBombStruct.bombImage[index[pTankEnemy->mBombStruct.bombCounter++]]),
						0, 0,
						BOX_SIZE * 2, BOX_SIZE * 2,
						0x000000);
				}
			}

			// 是否需要绘制敌机坦克炮弹
			if (pTankEnemy->mBullet.needDraw) {
				// 绘制玩家坦克炮弹
				int dir = pTankEnemy->mBullet.dir; // 炮弹方向
				int bulletX = pTankEnemy->mBullet.posX + pTankEnemy->mBullet.bullet_bias[dir][0]; // 将炮弹左上角的位置由坦克的中心移动到坦克炮管的位置
				int bulletY = pTankEnemy->mBullet.posY + pTankEnemy->mBullet.bullet_bias[dir][1];
				TransparentBlt(center_hdc,
					bulletX, bulletY,
					pTankEnemy->mBullet.bulletSize[dir][0], pTankEnemy->mBullet.bulletSize[dir][1],
					GetImageHDC(&pTankEnemy->mBullet.mImage[pTankEnemy->mBullet.dir]),
					0, 0,
					pTankEnemy->mBullet.bulletSize[dir][0], pTankEnemy->mBullet.bulletSize[dir][1],
					0x000000);
			}
		}

		// 敌机坦克死亡，绘制大爆炸效果
		if (pTankEnemy->mDied == true && pTankEnemy->mBlastStruct.showBlast == true) {
			int index[13] = { 0,1,1,2,2,3,3,4,4,3,2,1,0 };
			if (pTankEnemy->mBlastStruct.blastCounter >= 12) {
				if (pTankEnemy->mBlastStruct.blastCounter++ <= 18) {
					// 绘制敌机坦克分数图片
					TransparentBlt(center_hdc, 
						pTankEnemy->mBlastStruct.blastX - 7, pTankEnemy->mBlastStruct.blastY - 3,
						14, 7,
						GetImageHDC(&pTankEnemy->mScoreImage[pTankEnemy->mTankLevel]), 
						0, 0, 
						14, 7, 
						0x000000);
				}
				else {
					pTankEnemy->mBlastStruct.blastCounter = 0;
					pTankEnemy->mBlastStruct.showBlast = false;
				}							
			}
			else {
				TransparentBlt(center_hdc,
					pTankEnemy->mBlastStruct.blastX - BOX_SIZE*2, pTankEnemy->mBlastStruct.blastY - BOX_SIZE*2,
					BOX_SIZE * 4, BOX_SIZE * 4,
					GetImageHDC(&pTankEnemy->mBlastStruct.blastImage[index[pTankEnemy->mBlastStruct.blastCounter++]]),
					0, 0,
					BOX_SIZE * 4, BOX_SIZE * 4,
					0x000000);
			}

		}
	}
}

/**
	敌机坦克重新调整方向
*/
void tank_enemy_rejust_direction(TankEnemy* pTankEnemy) {
	if (pTankEnemy->dir == ENE_DIR_LEFT || pTankEnemy->dir == ENE_DIR_RIGHT) {
		bool val = rand() % 100 < 70;
		if (val) {
			pTankEnemy->dir = ENE_DIR_DOWN; // 设置敌机坦克大概率向下移动
		}
		else {
			pTankEnemy->dir = ENE_TANK_DIR(rand() % 4); // 强制将int转换成enum枚举类型
		}
	}
	else {
		pTankEnemy->dir = ENE_TANK_DIR(rand() % 4);
	}
}

/**
	判断敌机坦克是否可以通过某一个区域
	tankX和tankY是坦克下一步要移动的坐标，如果不能通行则坐标不变
*/
bool check_tank_enemy_can_pass(int dir, int tankX, int tankY) {	
	switch (dir) {
	case ENE_DIR_LEFT:
		tankX += -1;		
		break;
	case ENE_DIR_UP:
		tankY += -1;		
		break;
	case ENE_DIR_RIGHT:
		tankX += 1;
		break;
	case ENE_DIR_DOWN:
		tankY += 1;
		break;
	default:
		break;
	}

	int x1 = tankX - BOX_SIZE; // 计算将要移动新位置的坦克矩形左上角和右下角坐标
	int x2 = tankX + BOX_SIZE;
	int y1 = tankY - BOX_SIZE;
	int y2 = tankY + BOX_SIZE;

	for (int i = 0; i < 26; i++) {
		for (int j = 0; j < 26; j++) {
			if (map26x26[i][j] == _WALL ||
				map26x26[i][j] == _RIVER ||
				map26x26[i][j] == _STONE) { // 如果是墙、河流、石头
				int t_x1 = j * BOX_SIZE;
				int t_x2 = j * BOX_SIZE + BOX_SIZE;
				int t_y1 = i * BOX_SIZE;
				int t_y2 = i * BOX_SIZE + BOX_SIZE;
				// 是否地图的矩形和坦克的矩形满足不相交的条件（左右上下）:https://zhuanlan.zhihu.com/p/29704064
				bool nonIntersect = (x2 <= t_x1 + 1) ||
					(x1 >= t_x2 - 1) ||
					(y2 <= t_y1 + 1) ||
					(y1 >= t_y2 - 1);
				if (nonIntersect == false) {
					return false; // 说明两个矩形会相交
				}
			}
		}
	}

	// 判断是否超过游戏地图边界
	if (x1 < 0 || x2 > CENTER_WIDTH || y1 < 0 || y2 > CENTER_HEIGHT) {
		return false;
	}

	// 检测敌机坦克是否与玩家坦克重合
	int player_x1 = tankPlayer0.tankPlayerX - BOX_SIZE; // 玩家坦克坐标
	int player_y1 = tankPlayer0.tankPlayerY - BOX_SIZE;
	int player_x2 = tankPlayer0.tankPlayerX + BOX_SIZE;
	int player_y2 = tankPlayer0.tankPlayerY + BOX_SIZE;
	bool check_result = (x2 <= player_x1) ||
					(x1 >= player_x2) ||
					(y2 <= player_y1) ||
					(y1 >= player_y2);
	if (check_result == false) { // 说明与玩家坦克相撞
		return false;
	}

	return true;
}

/**
	发射炮弹撞击障碍物，遇到瓷砖则只爆炸，遇到红砖则爆炸并消除
*/
void check_enemy_bullet_to_obstacle(TankEnemy* pTankEnemy) {
	if (pTankEnemy->mBullet.needDraw == false) { // 如果没发射子弹不需要继续判断
		return;
	}
	int bulletSize[4][2] = { {4, 3}, {3, 4}, {4, 3}, {3, 4} }; // 左右：4*3   上下：3*4   

	int dir = pTankEnemy->mBullet.dir;
	int x1 = pTankEnemy->mBullet.posX; // 炮弹左上角的坐标(x1, y1)
	int y1 = pTankEnemy->mBullet.posY;
	int x2 = x1 + bulletSize[dir][0];
	int y2 = y1 + bulletSize[dir][1];

	switch (dir) { // 扩大炮弹不同方向的横截面，这样可以同时消掉两个砖块
	case ENE_DIR_LEFT:
		y1 = y1 - bulletSize[dir][1];
		break;
	case ENE_DIR_UP:
		x1 = x1 - bulletSize[dir][0];
		break;
	case ENE_DIR_RIGHT:
		y1 = y1 - bulletSize[dir][1];
		break;
	case ENE_DIR_DOWN:
		x1 = x1 - bulletSize[dir][0];
		break;
	default:
		break;
	}

	bool nonIntersect = false;
	for (int i = 0; i < 26; i++) {
		for (int j = 0; j < 26; j++) {
			if (map26x26[i][j] == _WALL ||
				map26x26[i][j] == _STONE) { // 如果是墙、石头	
				int t_x1 = j * BOX_SIZE;
				int t_x2 = j * BOX_SIZE + BOX_SIZE;
				int t_y1 = i * BOX_SIZE;
				int t_y2 = i * BOX_SIZE + BOX_SIZE;
				// 是否地图的矩形和炮弹的矩形满足不相交的条件（左右上下）:https://zhuanlan.zhihu.com/p/29704064
				nonIntersect = (x2 <= t_x1) ||
					(x1 >= t_x2) ||
					(y2 <= t_y1) ||
					(y1 >= t_y2); // ==true说明不相交  ==false说明相交

				if (nonIntersect == false) {
					pTankEnemy->mBombStruct.showBomb = true;	
					pTankEnemy->mBullet.needDraw = false; // 避免炮弹继续运动产生多次爆炸的BUG
					pTankEnemy->mBombStruct.mBombX = (x1 + x2) / 2;
					pTankEnemy->mBombStruct.mBombY = (y1 + y2) / 2;
					if (map26x26[i][j] == _WALL) { // 爆炸并设置清除标志
						map26x26[i][j] = _EMPTY; // 设置为空地
					}
				}
			}
		}
	}

	// 判断敌机炮弹是否击中玩家坦克
	if (tankPlayer0.mStar.starState != Star_Begin && tankPlayer0.mProtecCircle.needShow == false) { // 如果当前玩家还处于四角星或者重生保护帽模式，则不能被击毁
		int player_x1 = tankPlayer0.tankPlayerX - BOX_SIZE; // 玩家坦克坐标
		int player_y1 = tankPlayer0.tankPlayerY - BOX_SIZE;
		int player_x2 = tankPlayer0.tankPlayerX + BOX_SIZE;
		int player_y2 = tankPlayer0.tankPlayerY + BOX_SIZE;
		nonIntersect = (x2 <= player_x1) ||
			(x1 >= player_x2) ||
			(y2 <= player_y1) ||
			(y1 >= player_y2);
		if (nonIntersect == false) { // 说明炮弹击中玩家坦克		
			// 玩家坦克中心位置产生大爆炸
			tankPlayer0.mDied = true;
			tankPlayer0.mPlayerLife -= 1; // 生命值-1
			tankPlayer0.mBlastStruct.blastX = tankPlayer0.tankPlayerX;
			tankPlayer0.mBlastStruct.blastY = tankPlayer0.tankPlayerY;
			tankPlayer0.mBlastStruct.showBlast = true;

			PlaySounds(S_PLAYER_BOMB); // 玩家坦克爆炸音效
		}
	}

	// 判断是否击中大本营
	if (mCampDie == false) { // 如果大本营没有被击中才进行判断
		int camp_x1 = BOX_SIZE * 12;
		int camp_y1 = BOX_SIZE * 24;
		int camp_x2 = camp_x1 + BOX_SIZE * 2;
		int camp_y2 = camp_y1 + BOX_SIZE * 2;
		nonIntersect = (x2 <= camp_x1) ||
			(x1 >= camp_x2) ||
			(y2 <= camp_y1) ||
			(y1 >= camp_y2);
		if (nonIntersect == false) {
			mCampDie = true;
			showCampBomb = true;
			gameOverCounter = 0;
			gameOverX = CENTER_WIDTH / 2 - GAME_OVER_WIDTH / 2;
			gameOverY = CENTER_HEIGHT;
			PlaySounds(S_CAMP_BOMB);
		}
	}
}