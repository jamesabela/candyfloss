#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <wchar.h>
#include <string.h>

#define APP_W 960
#define APP_H 600
#define TIMER_ID 1
#define PI 3.14159265358979323846

typedef enum { SCREEN_TITLE, SCREEN_SETUP, SCREEN_PASS_TURN, SCREEN_SPIN, SCREEN_PLAN, SCREEN_RESULTS, SCREEN_FINAL, SCREEN_HOF, SCREEN_ANALYSIS } Screen;
typedef enum { WEATHER_SUNNY, WEATHER_BREEZY, WEATHER_CLOUDY, WEATHER_RAIN, WEATHER_HEAT, WEATHER_STORM } Weather;
typedef enum { SOUND_CLICK, SOUND_COIN, SOUND_PROFIT, SOUND_LOSS, SOUND_WIN } SoundType;
typedef enum { LANG_EN, LANG_KO } Language;

typedef enum {
    LOC_BLACKPOOL, LOC_SKEGNESS, LOC_CLACTON, LOC_BRIGHTON,
    LOC_JEJU, LOC_HAEUNDAE, LOC_GANGNEUNG
} Location;

typedef struct {
    const wchar_t *nameEN;
    const wchar_t *nameKO;
    const wchar_t *pierName;
    float crowdMult;
    float priceSens;
} LocationData;

static const LocationData locationsData[] = {
    { L"Blackpool", L"블랙풀", L"BLACKPOOL PIER", 1.15f, 1.00f },
    { L"Skegness", L"스케그니스", L"SKEGNESS PIER", 1.05f, 0.95f },
    { L"Clacton-on-Sea", L"클랙턴온시", L"CLACTON PIER", 1.00f, 0.90f },
    { L"Brighton Pier", L"브라이튼", L"BRIGHTON PIER", 1.10f, 1.25f },
    { L"Jeju Island", L"제주도", L"JEJU PARADISE", 1.20f, 1.10f },
    { L"Haeundae Beach", L"부산 해운대", L"HAEUNDAE BEACH", 1.30f, 1.05f },
    { L"Gangneung Beach", L"강릉 안목해변", L"GANGNEUNG BEACH", 1.08f, 1.15f }
};

typedef struct { RECT r; const wchar_t *text; int id; } UiButtonW;
typedef struct { RECT r; int minv, maxv, value; int id; } Slider;

typedef struct {
    int active;
    wchar_t businessName[32];
    int cashPence;
    int debtPence;
    int reputation;
    int stock, pricePence, adverts;
    int bonusStock;
    int sold, customers, unsold;
    int revenuePence, costPence, profitPence;
    int totalSold, totalStockMade, bestProfit;
    int stallColour;
} PlayerState;

typedef struct {
    wchar_t pName[32];
    wchar_t bName[32];
    wchar_t locName[32];
    int score;
    int cashPence;
    int reputation;
    int totalSold;
} HighScoreEntry;

typedef struct {
    Screen screen;
    Language lang;
    Location loc;
    int numPlayers;
    int currentPlayer;
    int day, totalDays;
    int difficulty;
    int soundOn;
    int unitCostPence, suggestedPricePence, crowd;
    Weather weather;
    int eventId;
    PlayerState players[4];
    HighScoreEntry highScores[5];
    int editingNamePlayer;
    
    // Spinning Mini-Game State
    DWORD spinStartTick;
    double lastMouseAngle;
    double spinTotalAngle;
    int bonusEarned;

    int mouseDown;
    int activeSlider;
    unsigned tick;
} Game;

static Game g;
static HFONT fontTitle, fontLarge, fontMedium, fontSmall, fontTiny;
static HDC backDC;
static HBITMAP backBmp;
static int clientW = APP_W, clientH = APP_H;

static const COLORREF stallColours[] = {
    RGB(255, 84, 158), RGB(45, 164, 235), RGB(246, 192, 62), RGB(70, 205, 161)
};
static const wchar_t *playerNamesEN[] = {L"Player 1", L"Player 2", L"Player 3", L"Player 4"};
static const wchar_t *playerNamesKO[] = {L"플레이어 1", L"플레이어 2", L"플레이어 3", L"플레이어 4"};

static const wchar_t *defaultBizNamesEN[] = {
    L"Pink Cloud Floss", L"Ocean Breeze Sweets", L"Golden Sun Sweets", L"Mint Pier Paradise",
    L"Beachfront Candy Co.", L"Royal Pier Treats"
};
static const wchar_t *defaultBizNamesKO[] = {
    L"분홍 구름 솜사탕", L"바다 바람 솜사탕", L"황금 햇살 솜사탕", L"민트 피어 솜사탕",
    L"해변길 솜사탕 상회", L"로얄 피어 디저트"
};

static const wchar_t *beachZoneNamesEN[] = {
    L"North Promenade", L"South Esplanade", L"East Dunes Plaza", L"West Coast Strip"
};
static const wchar_t *beachZoneNamesKO[] = {
    L"북쪽 피어 산책로", L"남쪽 해변 산책로", L"동쪽 언덕 광장", L"서쪽 해안 거리"
};

static const wchar_t *weatherNamesEN[] = {L"Sunny", L"Warm and breezy", L"Cloudy", L"Light rain", L"Heatwave", L"Storm"};
static const wchar_t *weatherNamesKO[] = {L"맑음", L"화창하고 바람", L"흐림", L"가벼운 비", L"폭염", L"폭풍우"};
static const char *weatherIcons[] = {"SUN", "BREEZE", "CLOUD", "RAIN", "HOT", "STORM"};
static const float weatherDemand[] = {1.18f, 1.05f, 0.86f, 0.68f, 1.48f, 0.42f};

static const wchar_t *eventNamesEN[] = {L"Normal day", L"School trip", L"Donkey trouble", L"Seagull invasion", L"Festival parade", L"Sugar delay", L"Perfect reviews"};
static const wchar_t *eventNamesKO[] = {L"평범한 하루", L"수학여행 단체 방문", L"장난꾸러기 당나귀", L"갈매기떼의 습격", L"축제 퍼레이드", L"설탕 배송 지연", L"최고의 맛집 리뷰"};

static const wchar_t *eventTextsEN[] = {
    L"Nothing unusual happens. Your decisions do the talking.",
    L"A coach full of pupils arrives at lunchtime.",
    L"A mischievous donkey eats some prepared stock.",
    L"Seagulls frighten customers away for part of the afternoon.",
    L"A colourful parade brings extra visitors past your stall.",
    L"A late sugar delivery raises production costs.",
    L"A local reviewer praises your candyfloss."
};
static const wchar_t *eventTextsKO[] = {
    L"특별한 일 없이 평범한 하루입니다. 경영 전략에 집중하세요.",
    L"점심시간에 학생들을 태운 버스가 도착했습니다.",
    L"장난꾸러기 당나귀가 준비해둔 재고 일부를 먹어치웠습니다.",
    L"갈매기떼가 나타나 오후 동안 손님들을 쫓아냈습니다.",
    L"화려한 퍼레이드로 많은 관람객이 가게 앞을 지나갑니다.",
    L"설탕 배송이 지연되어 원가 비용이 상승했습니다.",
    L"지역 맛집 평가원이 당신의 솜사탕을 극찬했습니다."
};

static int irand(int a, int b) { return a + rand() % (b - a + 1); }
static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static int ptIn(RECT r, int x, int y) { POINT p = {x,y}; return PtInRect(&r, p); }
static COLORREF lighten(COLORREF c, int n) {
    int r = clampi(GetRValue(c)+n,0,255), gg = clampi(GetGValue(c)+n,0,255), b = clampi(GetBValue(c)+n,0,255);
    return RGB(r,gg,b);
}

static void selectFontW(HDC dc, HFONT f) { SelectObject(dc, f); SetBkMode(dc, TRANSPARENT); }
static void textW(HDC dc, int x, int y, const wchar_t *s, COLORREF c, HFONT f) {
    selectFontW(dc, f); SetTextColor(dc, c); TextOutW(dc, x, y, s, (int)wcslen(s));
}
static void centreTextW(HDC dc, RECT r, const wchar_t *s, COLORREF c, HFONT f) {
    selectFontW(dc, f); SetTextColor(dc, c); DrawTextW(dc, s, -1, &r, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}
static void wrappedTextW(HDC dc, RECT r, const wchar_t *s, COLORREF c, HFONT f, UINT flags) {
    selectFontW(dc, f); SetTextColor(dc, c); DrawTextW(dc, s, -1, &r, flags|DT_WORDBREAK);
}
static void fillRound(HDC dc, RECT r, int radius, COLORREF c) {
    HBRUSH b=CreateSolidBrush(c); HPEN p=CreatePen(PS_SOLID,1,c); HGDIOBJ ob=SelectObject(dc,b), op=SelectObject(dc,p);
    RoundRect(dc,r.left,r.top,r.right,r.bottom,radius,radius);
    SelectObject(dc,ob); SelectObject(dc,op); DeleteObject(b); DeleteObject(p);
}
static void line(HDC dc,int x1,int y1,int x2,int y2,COLORREF c,int width) {
    HPEN p=CreatePen(PS_SOLID,width,c); HGDIOBJ old=SelectObject(dc,p); MoveToEx(dc,x1,y1,NULL); LineTo(dc,x2,y2); SelectObject(dc,old); DeleteObject(p);
}
static void ellipseFill(HDC dc,int l,int t,int r,int b,COLORREF c) {
    HBRUSH br=CreateSolidBrush(c); HPEN p=CreatePen(PS_SOLID,1,c); HGDIOBJ ob=SelectObject(dc,br),op=SelectObject(dc,p); Ellipse(dc,l,t,r,b); SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(br);DeleteObject(p);
}
static void polygonFill(HDC dc, POINT *pts, int n, COLORREF c) {
    HBRUSH br=CreateSolidBrush(c); HPEN p=CreatePen(PS_SOLID,1,c); HGDIOBJ ob=SelectObject(dc,br),op=SelectObject(dc,p); Polygon(dc,pts,n); SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(br);DeleteObject(p);
}
static void moneyW(wchar_t *buf, size_t n, int pence) {
    int neg=pence<0; if(neg)pence=-pence;
    swprintf(buf,n,L"%sGBP %d.%02d",neg?L"-":L"",pence/100,pence%100);
}

// Hall of Fame Save/Load
static void loadHighScores(void) {
    FILE *f = fopen("candyfloss.dat", "rb");
    if(f){
        fread(g.highScores, sizeof(HighScoreEntry), 5, f);
        fclose(f);
    } else {
        ZeroMemory(g.highScores, sizeof(g.highScores));
    }
}

static void saveHighScores(void) {
    FILE *f = fopen("candyfloss.dat", "wb");
    if(f){
        fwrite(g.highScores, sizeof(HighScoreEntry), 5, f);
        fclose(f);
    }
}

static void checkAndAddHighScore(const wchar_t *pName, const wchar_t *bName, const wchar_t *locName, int score, int cashPence, int rep, int totalSold) {
    int insertIdx = -1;
    for(int i=0;i<5;i++){
        if(score > g.highScores[i].score){
            insertIdx = i;
            break;
        }
    }
    if(insertIdx >= 0){
        for(int i=4; i>insertIdx; i--){
            g.highScores[i] = g.highScores[i-1];
        }
        wcsncpy(g.highScores[insertIdx].pName, pName, 31);
        wcsncpy(g.highScores[insertIdx].bName, bName, 31);
        wcsncpy(g.highScores[insertIdx].locName, locName, 31);
        g.highScores[insertIdx].score = score;
        g.highScores[insertIdx].cashPence = cashPence;
        g.highScores[insertIdx].reputation = rep;
        g.highScores[insertIdx].totalSold = totalSold;
        saveHighScores();
    }
}

// Performance Analysis Engine
static void analyzePlayerPerformance(int pIdx, wchar_t *gradeOut, wchar_t *titleOut, wchar_t *tipOut, int *effOut) {
    PlayerState *p = &g.players[pIdx];
    int netWealth = p->cashPence - p->debtPence;
    int score = netWealth + p->reputation * 45 + p->totalSold * 5;
    int eff = p->totalStockMade > 0 ? (int)((double)p->totalSold / p->totalStockMade * 100.0) : 0;
    if(effOut) *effOut = eff;

    if(score >= 12000 && netWealth >= 8000){
        wcscpy(gradeOut, L"S");
        wcscpy(titleOut, g.lang==LANG_EN ? L"Master Tycoon" : L"전설의 마스터 타이쿤");
    } else if(score >= 9000){
        wcscpy(gradeOut, L"A");
        wcscpy(titleOut, g.lang==LANG_EN ? L"Seaside Executive" : L"해변의 특급 경영자");
    } else if(score >= 6000){
        wcscpy(gradeOut, L"B");
        wcscpy(titleOut, g.lang==LANG_EN ? L"Profitable Trader" : L"수익형 상인");
    } else if(score >= 3000){
        wcscpy(gradeOut, L"C");
        wcscpy(titleOut, g.lang==LANG_EN ? L"Struggling Vendor" : L"노력하는 노점상");
    } else {
        wcscpy(gradeOut, L"D");
        wcscpy(titleOut, g.lang==LANG_EN ? L"Bankrupt Vendor" : L"파산 위기 노점상");
    }

    if(p->debtPence > 3000){
        wcscpy(tipOut, g.lang==LANG_EN ? L"High debt interest ate your profits. Repay bank loans early!" : L"대출 이자가 수익을 많이 깎아먹었습니다. 대출금을 조기에 상환하세요!");
    } else if(eff < 65){
        wcscpy(tipOut, g.lang==LANG_EN ? L"High unsold stock waste. Reduce production on rainy or storm days!" : L"남은 재고 낭비가 컸습니다. 비나 폭풍우가 치는 날에는 제조량을 줄이세요!");
    } else if(p->totalSold < 150){
        wcscpy(tipOut, g.lang==LANG_EN ? L"Sales volume was low. Try buying more advertising signs!" : L"판매량이 적었습니다. 홍보 표지판을 늘려 고객 유동인구를 유인하세요!");
    } else {
        wcscpy(tipOut, g.lang==LANG_EN ? L"Excellent balance of pricing, advertising, and inventory control!" : L"가격 설정, 홍보, 재고 관리의 훌륭한 경영 균형을 보여주었습니다!");
    }
}

static DWORD WINAPI audioThread(LPVOID param) {
    SoundType st=(SoundType)(uintptr_t)param;
    if(st==SOUND_CLICK){
        Beep(900, 30);
    } else if(st==SOUND_COIN){
        Beep(987, 40); Beep(1318, 90);
    } else if(st==SOUND_PROFIT){
        Beep(523, 70); Beep(659, 70); Beep(784, 70); Beep(1046, 140);
    } else if(st==SOUND_LOSS){
        Beep(440, 90); Beep(370, 90); Beep(310, 160);
    } else if(st==SOUND_WIN){
        Beep(523, 80); Beep(659, 80); Beep(784, 80); Beep(1046, 120); Beep(880, 80); Beep(1046, 250);
    }
    return 0;
}

static void playSound(SoundType st) {
    if(!g.soundOn) return;
    HANDLE hThread = CreateThread(NULL, 0, audioThread, (LPVOID)(uintptr_t)st, 0, NULL);
    if(hThread) CloseHandle(hThread);
}

static void drawCloud(HDC dc,int x,int y,int s,COLORREF c) {
    ellipseFill(dc,x,y+18*s,x+50*s,y+48*s,c);
    ellipseFill(dc,x+25*s,y,x+75*s,y+45*s,c);
    ellipseFill(dc,x+55*s,y+15*s,x+100*s,y+48*s,c);
    RECT r={x+10*s,y+25*s,x+90*s,y+50*s}; HBRUSH br=CreateSolidBrush(c); FillRect(dc,&r,br); DeleteObject(br);
}
static void drawSun(HDC dc,int x,int y,int r) {
    for(int i=0;i<12;i++){ double a=i*PI/6.0; line(dc,x+(int)(cos(a)*r*1.25),y+(int)(sin(a)*r*1.25),x+(int)(cos(a)*r*1.65),y+(int)(sin(a)*r*1.65),RGB(255,190,35),3); }
    ellipseFill(dc,x-r,y-r,x+r,y+r,RGB(255,213,66));
}
static void drawCandy(HDC dc,int x,int y,COLORREF c) {
    line(dc,x,y+30,x,y+93,RGB(176,119,65),5);
    for(int i=0;i<10;i++){ double a=i*PI*2/10.0; int px=x+(int)(cos(a)*17), py=y+(int)(sin(a)*20); ellipseFill(dc,px-15,py-15,px+15,py+15, i%2?lighten(c,20):c); }
    ellipseFill(dc,x-20,y-20,x+20,y+20,c);
}
static void drawPerson(HDC dc,int x,int y,COLORREF shirt,int phase) {
    int bob=(int)(sin((g.tick+phase)*0.08)*3.0);
    y+=bob;
    ellipseFill(dc,x-13,y-72,x+13,y-46,RGB(239,188,145));
    ellipseFill(dc,x-14,y-76,x+14,y-60,RGB(81,50,35));
    RECT body={x-16,y-47,x+16,y}; fillRound(dc,body,10,shirt);
    line(dc,x-7,y,x-9,y+30,RGB(42,65,98),5); line(dc,x+7,y,x+9,y+30,RGB(42,65,98),5);
}
static void drawSeagull(HDC dc,int x,int y,int s) {
    HPEN p=CreatePen(PS_SOLID,3,RGB(245,248,250)); HGDIOBJ old=SelectObject(dc,p);
    Arc(dc,x,y,x+30*s,y+18*s, x,y+7*s,x+15*s,y+15*s);
    Arc(dc,x+28*s,y,x+58*s,y+18*s, x+43*s,y+15*s,x+58*s,y+7*s);
    SelectObject(dc,old);DeleteObject(p);
}
static void drawScene(HDC dc) {
    COLORREF sky = g.weather==WEATHER_STORM?RGB(100,112,139):(g.weather==WEATHER_RAIN?RGB(154,190,209):RGB(105,207,247));
    RECT r={0,0,640,470}; HBRUSH b=CreateSolidBrush(sky);FillRect(dc,&r,b);DeleteObject(b);
    r=(RECT){0,280,640,390}; b=CreateSolidBrush(RGB(37,160,210));FillRect(dc,&r,b);DeleteObject(b);
    r=(RECT){0,390,640,520}; b=CreateSolidBrush(RGB(231,199,145));FillRect(dc,&r,b);DeleteObject(b);
    if(g.weather==WEATHER_SUNNY||g.weather==WEATHER_BREEZY||g.weather==WEATHER_HEAT) drawSun(dc,545,75,g.weather==WEATHER_HEAT?38:28);
    drawCloud(dc,55+(int)(sin(g.tick*.015)*12),55,1,RGB(250,250,250));
    drawCloud(dc,310+(int)(sin(g.tick*.011)*18),38,1,RGB(248,248,248));
    for(int i=0;i<8;i++){ int xx=((int)g.tick*2+i*100)%760-80; line(dc,xx,330,xx+45,330+(i%2?3:-3),RGB(210,244,250),3); }
    // pier
    r=(RECT){390,230,620,245}; b=CreateSolidBrush(RGB(92,94,108));FillRect(dc,&r,b);DeleteObject(b);
    for(int x=410;x<620;x+=40) line(dc,x,245,x,350,RGB(85,83,91),5);
    r=(RECT){460,180,605,230}; b=CreateSolidBrush(RGB(250,239,244));FillRect(dc,&r,b);DeleteObject(b);
    r=(RECT){450,169,615,184}; b=CreateSolidBrush(RGB(235,74,139));FillRect(dc,&r,b);DeleteObject(b);
    centreTextW(dc,(RECT){455,183,610,206},locationsData[g.loc].pierName,RGB(35,59,98),fontSmall);
    const wchar_t *zName = g.lang==LANG_EN ? beachZoneNamesEN[g.currentPlayer] : beachZoneNamesKO[g.currentPlayer];
    centreTextW(dc,(RECT){455,206,610,227},zName,RGB(110,125,145),fontTiny);
    
    // stall
    COLORREF sc=stallColours[g.players[g.currentPlayer].stallColour];
    r=(RECT){45,235,330,465}; b=CreateSolidBrush(lighten(sc,130));FillRect(dc,&r,b);DeleteObject(b);
    for(int i=0;i<6;i++){ RECT aw={35+i*52,205,87+i*52,248}; b=CreateSolidBrush(i%2?RGB(255,247,239):sc);FillRect(dc,&aw,b);DeleteObject(b); }
    r=(RECT){35,390,350,415}; b=CreateSolidBrush(RGB(126,79,50));FillRect(dc,&r,b);DeleteObject(b);
    r=(RECT){70,145,320,205}; fillRound(dc,r,18,RGB(255,255,255));
    
    // Custom Player Business Name on Stall Sign
    const wchar_t *bName = g.players[g.currentPlayer].businessName;
    centreTextW(dc,(RECT){75,150,315,180},bName,sc,fontMedium);
    centreTextW(dc,(RECT){75,176,315,198},g.lang==LANG_EN?L"CANDYFLOSS STALL":L"솜사탕 가게",RGB(39,63,104),fontTiny);
    
    // seller
    ellipseFill(dc,120,285,164,329,RGB(239,188,145)); ellipseFill(dc,118,277,166,304,RGB(78,45,31));
    r=(RECT){107,326,177,405};fillRound(dc,r,14,sc);
    // machine
    ellipseFill(dc,205,330,315,390,RGB(205,232,242)); ellipseFill(dc,225,342,295,376,RGB(255,143,190));
    drawCandy(dc,194,275,RGB(255,130,184));
    // customers
    drawPerson(dc,405,430,RGB(76,182,239),0); drawPerson(dc,475,440,RGB(242,116,161),15); drawPerson(dc,550,425,RGB(111,205,134),30);
    // bunting
    line(dc,10,120,630,145,RGB(255,255,255),2);
    COLORREF flags[]={RGB(255,80,153),RGB(255,211,75),RGB(42,169,242),RGB(79,210,173)};
    for(int i=0;i<13;i++){int x=20+i*48,y=120+i*2;POINT p[3]={{x,y},{x+18,y+1},{x+9,y+22}};polygonFill(dc,p,3,flags[i%4]);}
    drawSeagull(dc,20+(g.tick*2)%700,80,1);
    if(g.weather==WEATHER_RAIN||g.weather==WEATHER_STORM){ int count=g.weather==WEATHER_STORM?45:25; for(int i=0;i<count;i++){int x=(i*71+g.tick*5)%640,y=(i*43+g.tick*8)%470;line(dc,x,y,x-7,y+17,RGB(220,244,252),2);} }
}

static void drawButtonW(HDC dc, UiButtonW b, COLORREF colour) {
    fillRound(dc,b.r,24,RGB(255,255,255)); RECT in={b.r.left+3,b.r.top+3,b.r.right-3,b.r.bottom-5}; fillRound(dc,in,22,colour); centreTextW(dc,in,b.text,RGB(255,255,255),fontMedium);
}
static void drawSliderW(HDC dc, Slider s, const wchar_t *label, const wchar_t *value) {
    textW(dc,s.r.left,s.r.top-28,label,RGB(34,54,90),fontSmall); RECT vr={s.r.right-120,s.r.top-30,s.r.right,s.r.top-4}; centreTextW(dc,vr,value,RGB(201,42,112),fontSmall);
    int cy=(s.r.top+s.r.bottom)/2; line(dc,s.r.left,cy,s.r.right,cy,RGB(216,225,235),10);
    int x=s.r.left+(int)((double)(s.value-s.minv)/(s.maxv-s.minv)*(s.r.right-s.r.left)); line(dc,s.r.left,cy,x,cy,RGB(255,75,151),10); ellipseFill(dc,x-13,cy-13,x+13,cy+13,RGB(255,255,255)); ellipseFill(dc,x-8,cy-8,x+8,cy+8,RGB(255,75,151));
}

static UiButtonW titleButtons[] = {
    {{360,210,600,260},L"PLAY GAME",1},
    {{360,272,600,322},L"HOW TO PLAY",2},
    {{360,334,600,384},L"HALL OF FAME",3},
    {{360,396,600,446},L"SOUND: ON",4},
    {{360,458,600,508},L"LANG: EN",5}
};
static UiButtonW setupButtons[] = {
    {{330,515,630,570},L"START GAME",10}, {{30,30,140,68},L"BACK",11}
};
static UiButtonW passButtons[] = {
    {{320,420,640,480},L"START FLOSS SPINNING",15}
};
static UiButtonW planButtons[] = {
    {{700,505,920,565},L"OPEN FOR BUSINESS",20},
    {{650,425,780,460},L"BORROW £50",21},
    {{790,425,920,460},L"REPAY £50",22}
};
static UiButtonW resultButtons[] = { {{340,510,620,565},L"NEXT PLAYER",30} };
static UiButtonW finalButtons[] = {
    {{230,515,460,570},L"PLAY AGAIN",40},
    {{500,515,730,570},L"PERFORMANCE REPORT",41}
};
static UiButtonW hofButtons[] = { {{360,515,600,570},L"BACK TO TITLE",50} };
static UiButtonW analysisButtons[] = { {{360,520,600,572},L"BACK TO LEADERBOARD",60} };

static Slider sliders[] = {
    {{685,215,915,245},10,250,80,1},
    {{685,295,915,325},100,500,250,2},
    {{685,375,915,405},0,20,3,3}
};

static void drawFlagUK(HDC dc, int x, int y, int w, int h) {
    RECT r = {x, y, x + w, y + h};
    fillRound(dc, r, 2, RGB(1, 33, 105));
    line(dc, x, y, x + w, y + h, RGB(255, 255, 255), 3);
    line(dc, x + w, y, x, y + h, RGB(255, 255, 255), 3);
    line(dc, x, y, x + w, y + h, RGB(200, 16, 46), 1);
    line(dc, x + w, y, x, y + h, RGB(200, 16, 46), 1);
    int cx = x + w / 2, cy = y + h / 2;
    line(dc, cx, y, cx, y + h, RGB(255, 255, 255), 5);
    line(dc, x, cy, x + w, cy, RGB(255, 255, 255), 5);
    line(dc, cx, y, cx, y + h, RGB(200, 16, 46), 2);
    line(dc, x, cy, x + w, cy, RGB(200, 16, 46), 2);
    FrameRect(dc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
}

static void drawFlagKO(HDC dc, int x, int y, int w, int h) {
    RECT r = {x, y, x + w, y + h};
    fillRound(dc, r, 2, RGB(255, 255, 255));
    int cx = x + w / 2, cy = y + h / 2, cr = h / 3;
    ellipseFill(dc, cx - cr, cy - cr, cx + cr, cy + cr, RGB(5, 75, 150));
    HBRUSH redBr = CreateSolidBrush(RGB(205, 46, 58)); HPEN redPen = CreatePen(PS_SOLID, 1, RGB(205, 46, 58));
    HGDIOBJ ob = SelectObject(dc, redBr), op = SelectObject(dc, redPen);
    Pie(dc, cx - cr, cy - cr, cx + cr, cy + cr, cx + cr, cy, cx - cr, cy);
    SelectObject(dc, ob); SelectObject(dc, op); DeleteObject(redBr); DeleteObject(redPen);
    ellipseFill(dc, cx - cr, cy - cr/2, cx, cy + cr/2, RGB(205, 46, 58));
    ellipseFill(dc, cx, cy - cr/2, cx + cr, cy + cr/2, RGB(5, 75, 150));
    
    COLORREF blk = RGB(30, 30, 30);
    line(dc, x + 2, y + 2, x + 6, y + 2, blk, 1);
    line(dc, x + 2, y + 4, x + 6, y + 4, blk, 1);
    line(dc, x + 2, y + 6, x + 6, y + 6, blk, 1);
    line(dc, x + w - 6, y + h - 6, x + w - 2, y + h - 6, blk, 1);
    line(dc, x + w - 6, y + h - 4, x + w - 2, y + h - 4, blk, 1);
    line(dc, x + w - 6, y + h - 2, x + w - 2, y + h - 2, blk, 1);
    FrameRect(dc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
}

static void drawTitle(HDC dc) {
    RECT skyR={0,0,960,420}; HBRUSH sb=CreateSolidBrush(RGB(85,195,245)); FillRect(dc,&skyR,sb); DeleteObject(sb);
    RECT seaR={0,420,960,500}; HBRUSH mb=CreateSolidBrush(RGB(35,150,210)); FillRect(dc,&seaR,mb); DeleteObject(mb);
    RECT sandR={0,500,960,600}; HBRUSH ab=CreateSolidBrush(RGB(240,205,145)); FillRect(dc,&sandR,ab); DeleteObject(ab);

    drawSun(dc,840,85,36);
    drawCloud(dc,60+(int)(sin(g.tick*.015)*15),45,1,RGB(255,255,255));
    drawCloud(dc,680+(int)(sin(g.tick*.011)*20),30,1,RGB(255,255,255));
    drawCloud(dc,380+(int)(sin(g.tick*.013)*12),60,1,RGB(255,255,255));

    for(int i=0;i<10;i++){
        int xx=((int)g.tick*2+i*110)%1080-60;
        line(dc,xx,450,xx+50,450+(i%2?3:-3),RGB(210,244,255),3);
    }

    line(dc,0,15,960,30,RGB(255,255,255),2);
    COLORREF flags[]={RGB(255,80,153),RGB(255,211,75),RGB(42,169,242),RGB(79,210,173)};
    for(int i=0;i<20;i++){
        int x=10+i*48,y=15+(i%2)*4;
        POINT p[3]={{x,y},{x+20,y+1},{x+10,y+25}};
        polygonFill(dc,p,3,flags[i%4]);
    }

    drawCandy(dc,140,80,RGB(255,105,180));
    drawCandy(dc,820,80,RGB(50,195,235));

    const wchar_t *t1 = g.lang==LANG_EN ? L"CANDYFLOSS" : L"솜사탕";
    const wchar_t *t2 = g.lang==LANG_EN ? L"TYCOON" : L"타이쿤";
    const wchar_t *sub = g.lang==LANG_EN ? L"★ 1-4 PLAYER FLOPPY EDITION ★" : L"★ 1-4인용 플로피 에디션 ★";

    centreTextW(dc,(RECT){133,30,833,100},t1,RGB(120,20,65),fontTitle);
    centreTextW(dc,(RECT){130,27,830,97},t1,RGB(255,255,255),fontTitle);

    centreTextW(dc,(RECT){253,98,713,158},t2,RGB(160,110,0),fontLarge);
    centreTextW(dc,(RECT){250,95,710,155},t2,RGB(255,215,0),fontLarge);

    RECT badge={280,155,680,195};
    fillRound(dc,badge,20,RGB(34,55,95));
    centreTextW(dc,badge,sub,RGB(255,235,130),fontSmall);

    titleButtons[0].text = g.lang==LANG_EN ? L"PLAY GAME" : L"게임 시작";
    titleButtons[1].text = g.lang==LANG_EN ? L"HOW TO PLAY" : L"게임 설명";
    titleButtons[2].text = g.lang==LANG_EN ? L"HALL OF FAME" : L"명예의 전당";
    titleButtons[3].text = g.soundOn ? (g.lang==LANG_EN?L"SOUND: ON":L"소리: 켜짐") : (g.lang==LANG_EN?L"SOUND: OFF":L"소리: 꺼짐");
    titleButtons[4].text = g.lang==LANG_EN ? L"English / 한국어" : L"한국어 / English";

    for(int i=0;i<5;i++){
        COLORREF btnCol=(i==0)?RGB(255,50,135):(i==1?RGB(40,160,235):(i==2?RGB(235,160,30):(i==3?(g.soundOn?RGB(40,190,120):RGB(235,140,40)):RGB(140,90,210))));
        drawButtonW(dc,titleButtons[i],btnCol);
        if(i==4){
            drawFlagUK(dc, 372, 472, 28, 18);
            drawFlagKO(dc, 560, 472, 28, 18);
            if(g.lang==LANG_EN){
                RECT hl = {370, 470, 402, 492};
                FrameRect(dc, &hl, (HBRUSH)GetStockObject(WHITE_BRUSH));
            } else {
                RECT hl = {558, 470, 590, 492};
                FrameRect(dc, &hl, (HBRUSH)GetStockObject(WHITE_BRUSH));
            }
        }
    }
}

static void drawSetup(HDC dc) {
    RECT all={0,0,960,600}; HBRUSH b=CreateSolidBrush(RGB(222,247,255));FillRect(dc,&all,b);DeleteObject(b);
    centreTextW(dc,(RECT){160,15,800,65},g.lang==LANG_EN?L"GAME SETUP":L"게임 설정",RGB(30,55,96),fontLarge);
    
    // Location Selector
    textW(dc,60,75,g.lang==LANG_EN?L"Seaside Location":L"해변 지역 선택",RGB(35,55,90),fontSmall);
    for(int i=0;i<7;i++){
        RECT r={60+i*122,105,175+i*122,145};
        fillRound(dc,r,14,g.loc==i?RGB(255,75,151):RGB(190,219,235));
        const wchar_t *lName = g.lang==LANG_EN ? locationsData[i].nameEN : locationsData[i].nameKO;
        centreTextW(dc,r,lName,RGB(255,255,255),fontTiny);
    }

    textW(dc,60,165,g.lang==LANG_EN?L"Number of players":L"플레이어 수",RGB(35,55,90),fontSmall);
    const wchar_t *pCountEN[] = {L"1 PLAYER", L"2 PLAYERS", L"3 PLAYERS", L"4 PLAYERS"};
    const wchar_t *pCountKO[] = {L"1인용", L"2인용", L"3인용", L"4인용"};
    for(int i=0;i<4;i++){
        RECT r={60+i*140,195,185+i*140,235};
        fillRound(dc,r,16,g.numPlayers==(i+1)?RGB(255,75,151):RGB(190,219,235));
        centreTextW(dc,r,g.lang==LANG_EN?pCountEN[i]:pCountKO[i],RGB(255,255,255),fontTiny);
    }

    textW(dc,640,165,g.lang==LANG_EN?L"Game length & Difficulty":L"기간 및 난이도",RGB(35,55,90),fontSmall);
    const wchar_t *daysEN[] = {L"7 DAYS",L"10 DAYS",L"14 DAYS"};
    const wchar_t *daysKO[] = {L"7일간",L"10일간",L"14일간"};
    for(int i=0;i<3;i++){
        RECT r={630+i*100,195,720+i*100,235};
        fillRound(dc,r,16,g.totalDays==(i==0?7:i==1?10:14)?RGB(42,166,232):RGB(190,219,235));
        centreTextW(dc,r,g.lang==LANG_EN?daysEN[i]:daysKO[i],RGB(255,255,255),fontTiny);
    }

    // Business Name Typing Text Boxes
    textW(dc,60,250,g.lang==LANG_EN?L"Business Names (Click box to type custom name)":L"가게 상호명 (상자를 클릭하여 직접 입력 가능)",RGB(35,55,90),fontSmall);
    for(int i=0;i<g.numPlayers;i++){
        RECT r={60,285+i*52,500,325+i*52};
        fillRound(dc,r,14,g.editingNamePlayer==i?RGB(255,240,200):RGB(255,255,255));
        FrameRect(dc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));
        
        wchar_t tag[64];
        const wchar_t *pName = g.lang==LANG_EN ? playerNamesEN[i] : playerNamesKO[i];
        swprintf(tag,64,L"%s:",pName);
        textW(dc,75,295+i*52,tag,stallColours[g.players[i].stallColour],fontSmall);
        
        wchar_t dispName[64];
        swprintf(dispName,64,L"%s%s",g.players[i].businessName,g.editingNamePlayer==i?L"|":L"");
        textW(dc,185,295+i*52,dispName,RGB(30,50,90),fontSmall);

        // Preset button
        RECT pr={515,285+i*52,615,325+i*52};
        fillRound(dc,pr,14,RGB(190,219,235));
        centreTextW(dc,pr,g.lang==LANG_EN?L"PRESET":L"추천",RGB(255,255,255),fontTiny);
    }

    if(g.numPlayers==1){
        textW(dc,640,250,g.lang==LANG_EN?L"Stall Colour":L"점포 색상",RGB(35,55,90),fontSmall);
        for(int i=0;i<4;i++){
            RECT r={640+i*70,285,700+i*70,325};
            fillRound(dc,r,14,stallColours[i]);
            if(g.players[0].stallColour==i){ FrameRect(dc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH)); }
        }
    }

    setupButtons[0].text = g.lang==LANG_EN ? L"START GAME" : L"게임 시작";
    setupButtons[1].text = g.lang==LANG_EN ? L"BACK" : L"뒤로";

    drawButtonW(dc,setupButtons[0],RGB(244,58,139));
    drawButtonW(dc,setupButtons[1],RGB(43,163,232));
}

static void drawPassTurn(HDC dc) {
    RECT all={0,0,960,600}; HBRUSH b=CreateSolidBrush(RGB(42,163,232)); FillRect(dc,&all,b); DeleteObject(b);
    RECT p={150,70,810,510}; fillRound(dc,p,32,RGB(255,255,255));
    
    wchar_t buf[128];
    const wchar_t *zName = g.lang==LANG_EN ? beachZoneNamesEN[g.currentPlayer] : beachZoneNamesKO[g.currentPlayer];
    if(g.lang==LANG_EN) swprintf(buf,128,L"DAY %d / %d  •  %s (%s)",g.day,g.totalDays,locationsData[g.loc].nameEN,zName);
    else swprintf(buf,128,L"%d일차 / 총 %d일  •  %s (%s)",g.day,g.totalDays,locationsData[g.loc].nameKO,zName);
    centreTextW(dc,(RECT){200,95,760,135},buf,RGB(100,115,135),fontMedium);
    
    const wchar_t *pName = g.lang==LANG_EN ? playerNamesEN[g.currentPlayer] : playerNamesKO[g.currentPlayer];
    if(g.lang==LANG_EN) swprintf(buf,128,L"PASS THE MOUSE TO %s",pName);
    else swprintf(buf,128,L"%s 차례입니다 (마우스 전달)",pName);
    centreTextW(dc,(RECT){180,145,780,205},buf,stallColours[g.players[g.currentPlayer].stallColour],fontTitle);

    centreTextW(dc,(RECT){180,215,780,255},g.players[g.currentPlayer].businessName,RGB(35,55,95),fontLarge);

    wchar_t mbuf[64];
    moneyW(mbuf,64,g.players[g.currentPlayer].cashPence);
    if(g.lang==LANG_EN) swprintf(buf,128,L"Current Balance: %s  |  Reputation: %d",mbuf,g.players[g.currentPlayer].reputation);
    else swprintf(buf,128,L"현재 잔액: %s  |  평판: %d점",mbuf,g.players[g.currentPlayer].reputation);
    centreTextW(dc,(RECT){180,270,780,315},buf,RGB(45,65,95),fontMedium);

    const wchar_t *wName = g.lang==LANG_EN ? weatherNamesEN[g.weather] : weatherNamesKO[g.weather];
    if(g.lang==LANG_EN) swprintf(buf,128,L"Weather Today: %s  (Crowd: %d)",wName,g.crowd);
    else swprintf(buf,128,L"오늘 날씨: %s  (유동인구: %d명)",wName,g.crowd);
    centreTextW(dc,(RECT){180,325,780,365},buf,RGB(90,105,125),fontSmall);

    passButtons[0].text = g.lang==LANG_EN ? L"START FLOSS SPINNING!" : L"솜사탕 제조 시작!";
    drawButtonW(dc,passButtons[0],RGB(244,58,139));
}

// 5-second Mouse-Spinning Arcade Mini-Game
static void drawSpinningMiniGame(HDC dc) {
    RECT all={0,0,960,600}; HBRUSH b=CreateSolidBrush(RGB(30,40,75)); FillRect(dc,&all,b); DeleteObject(b);
    
    DWORD elapsed = GetTickCount() - g.spinStartTick;
    int remainingMs = 5000 - (int)elapsed;
    if(remainingMs < 0) remainingMs = 0;

    // Header
    const wchar_t *pName = g.lang==LANG_EN ? playerNamesEN[g.currentPlayer] : playerNamesKO[g.currentPlayer];
    wchar_t buf[128];
    swprintf(buf,128,L"%s  •  %s",pName,g.players[g.currentPlayer].businessName);
    centreTextW(dc,(RECT){100,20,860,60},buf,stallColours[g.players[g.currentPlayer].stallColour],fontLarge);

    const wchar_t *titleStr = g.lang==LANG_EN ? L"SWIRL MOUSE IN CIRCLES TO CRAFT BONUS FLOSS!" : L"마우스를 원형으로 빙글빙글 돌려 보너스 솜사탕을 만드세요!";
    centreTextW(dc,(RECT){100,65,860,100},titleStr,RGB(255,235,130),fontSmall);

    // Centered Machine Bowl
    int cx = 480, cy = 290;
    ellipseFill(dc, cx-170, cy-170, cx+170, cy+170, RGB(180, 205, 225));
    ellipseFill(dc, cx-150, cy-150, cx+150, cy+150, RGB(225, 240, 250));
    ellipseFill(dc, cx-70, cy-70, cx+70, cy+70, RGB(160, 185, 205));

    // Floss Cone growing with swirls
    COLORREF sc = stallColours[g.players[g.currentPlayer].stallColour];
    int flossSize = 15 + g.bonusEarned * 5;
    if(flossSize > 135) flossSize = 135;

    drawCandy(dc, cx, cy - 40, sc);
    for(int i=0;i<g.bonusEarned;i++){
        double a = i * PI * 0.4 + (g.tick * 0.1);
        int px = cx + (int)(cos(a) * (20 + i*4));
        int py = cy - 20 + (int)(sin(a) * (15 + i*3));
        ellipseFill(dc, px-20, py-20, px+20, py+20, i%2 ? lighten(sc, 40) : sc);
    }

    // Countdown ring and time
    wchar_t timeStr[32];
    swprintf(timeStr,32,L"00:0%d.%d", remainingMs/1000, (remainingMs%1000)/100);
    centreTextW(dc,(RECT){cx-100, cy+180, cx+100, cy+220}, timeStr, RGB(255, 255, 255), fontTitle);

    // Bonus Stock Counter
    swprintf(buf,128,g.lang==LANG_EN?L"BONUS FLOSS CRAFTED: +%d STICKS":L"획득한 보너스 솜사탕: +%d개", g.bonusEarned);
    centreTextW(dc,(RECT){100, 525, 860, 565}, buf, RGB(45, 215, 140), fontLarge);

    if(remainingMs <= 0){
        g.players[g.currentPlayer].bonusStock = g.bonusEarned;
        g.screen = SCREEN_PLAN;
        playSound(SOUND_WIN);
    }
}

static void drawHallOfFame(HDC dc) {
    RECT all={0,0,960,600}; HBRUSH b=CreateSolidBrush(RGB(35,45,85)); FillRect(dc,&all,b); DeleteObject(b);
    RECT p={100,45,860,565}; fillRound(dc,p,32,RGB(255,255,255));

    centreTextW(dc,(RECT){150,60,810,115},g.lang==LANG_EN?L"HALL OF FAME LEGENDS":L"명예의 전당 전설들",RGB(37,58,99),fontLarge);

    const wchar_t *ranks[]={L"1st",L"2nd",L"3rd",L"4th",L"5th"};
    COLORREF rankCols[]={RGB(235,170,30), RGB(130,140,155), RGB(185,110,50), RGB(90,105,125), RGB(90,105,125)};

    for(int i=0;i<5;i++){
        HighScoreEntry *e = &g.highScores[i];
        RECT rowR={140,125+i*72,820,187+i*72};
        fillRound(dc,rowR,18,i==0?RGB(255,248,220):RGB(242,247,252));

        RECT rankR={155,125+i*72,220,187+i*72};
        centreTextW(dc,rankR,ranks[i],rankCols[i],fontSmall);

        if(e->score == 0){
            RECT emptyR = {230, 125+i*72, 800, 187+i*72};
            centreTextW(dc, emptyR, g.lang==LANG_EN ? L"— Empty Slot —" : L"— 기록 없음 —", RGB(160, 175, 195), fontSmall);
            continue;
        }

        wchar_t buf[128];
        swprintf(buf,128,L"%s (%s)",e->pName,e->bName);
        textW(dc,230,135+i*72,buf,RGB(34,55,95),fontSmall);
        textW(dc,230,160+i*72,e->locName,RGB(110,125,145),fontTiny);

        moneyW(buf,128,e->cashPence);
        RECT cashR={440,125+i*72,580,187+i*72};
        centreTextW(dc,cashR,buf,RGB(25,139,98),fontSmall);

        swprintf(buf,128,L"Rep: %d",e->reputation);
        RECT repR={590,125+i*72,690,187+i*72};
        centreTextW(dc,repR,buf,RGB(40,150,220),fontSmall);

        swprintf(buf,128,L"Score: %d",e->score);
        RECT scoreR={700,125+i*72,810,187+i*72};
        centreTextW(dc,scoreR,buf,RGB(235,71,139),fontSmall);
    }

    hofButtons[0].text = g.lang==LANG_EN ? L"BACK TO TITLE" : L"메뉴로 돌아가기";
    drawButtonW(dc,hofButtons[0],RGB(244,58,139));
}

// End-of-Game Player Performance Analysis Screen
static void drawPerformanceAnalysis(HDC dc) {
    RECT all={0,0,960,600}; HBRUSH b=CreateSolidBrush(RGB(40,30,75)); FillRect(dc,&all,b); DeleteObject(b);
    RECT p={80,35,880,565}; fillRound(dc,p,32,RGB(255,255,255));

    centreTextW(dc,(RECT){120,48,840,95},g.lang==LANG_EN?L"PLAYER PERFORMANCE ANALYSIS":L"플레이어 경영 성과 분석",RGB(37,58,99),fontLarge);

    COLORREF gradeCols[]={RGB(235,170,30), RGB(35,160,220), RGB(40,180,110), RGB(225,130,40), RGB(210,40,75)};

    for(int i=0;i<g.numPlayers;i++){
        wchar_t grade[8], title[64], tip[256];
        int eff = 0;
        analyzePlayerPerformance(i, grade, title, tip, &eff);

        int rowY = 100 + i * 102;
        RECT cardR = {110, rowY, 850, rowY + 92};
        fillRound(dc, cardR, 18, RGB(244, 248, 252));

        // Grade Badge
        RECT gradeR = {125, rowY + 12, 195, rowY + 80};
        COLORREF gCol = (grade[0]==L'S')?gradeCols[0]:(grade[0]==L'A'?gradeCols[1]:(grade[0]==L'B'?gradeCols[2]:(grade[0]==L'C'?gradeCols[3]:gradeCols[4])));
        fillRound(dc, gradeR, 14, gCol);
        centreTextW(dc, gradeR, grade, RGB(255,255,255), fontTitle);

        // Player & Business Name + Title
        const wchar_t *pName = g.lang==LANG_EN ? playerNamesEN[i] : playerNamesKO[i];
        wchar_t tag[128];
        swprintf(tag, 128, L"%s - %s", pName, g.players[i].businessName);
        textW(dc, 210, rowY + 12, tag, stallColours[g.players[i].stallColour], fontSmall);

        swprintf(tag, 128, L"Award: %s", title);
        textW(dc, 210, rowY + 34, tag, RGB(35, 55, 95), fontTiny);

        // Efficiency %
        swprintf(tag, 128, g.lang==LANG_EN?L"Sales Efficiency: %d%%":L"판매 효율성: %d%%", eff);
        textW(dc, 640, rowY + 14, tag, RGB(25, 139, 98), fontSmall);

        // Strategic Business Tip
        textW(dc, 210, rowY + 56, tip, RGB(100, 110, 130), fontTiny);
    }

    analysisButtons[0].text = g.lang==LANG_EN ? L"BACK TO LEADERBOARD" : L"순위표로 돌아가기";
    drawButtonW(dc, analysisButtons[0], RGB(244,58,139));
}

static void prepareDay(void) {
    int roll=irand(1,100);
    g.weather=roll<=27?WEATHER_SUNNY:roll<=52?WEATHER_BREEZY:roll<=72?WEATHER_CLOUDY:roll<=87?WEATHER_RAIN:roll<=95?WEATHER_HEAT:WEATHER_STORM;
    
    float locMult = locationsData[g.loc].crowdMult;
    g.crowd=(int)((irand(82,145)+(g.day-1)*3)*locMult);
    g.unitCostPence=clampi(42+(g.day-1)*2+irand(-4,6),25,85);
    g.suggestedPricePence=(int)((235+(g.day-1)*5+irand(-12,12))*locationsData[g.loc].priceSens);
    g.currentPlayer=0;

    for(int i=0;i<g.numPlayers;i++){
        PlayerState *p=&g.players[i];
        p->bonusStock=0;
        p->stock=clampi(p->cashPence/g.unitCostPence,10,90);
        p->pricePence=((g.suggestedPricePence+5)/10)*10;
        p->adverts=p->cashPence>1800?3:0;
    }
}

static int upfrontCost(void){
    PlayerState *p=&g.players[g.currentPlayer];
    return p->stock*g.unitCostPence+p->adverts*150;
}

static void drawPlan(HDC dc) {
    PlayerState *p=&g.players[g.currentPlayer];
    RECT all={0,0,960,600};HBRUSH b=CreateSolidBrush(RGB(235,249,255));FillRect(dc,&all,b);DeleteObject(b);
    drawScene(dc);
    
    RECT right={640,0,960,600};b=CreateSolidBrush(RGB(255,255,255));FillRect(dc,&right,b);DeleteObject(b);
    
    wchar_t buf[128];
    // Line 1: Day & Cash (y = 12)
    swprintf(buf,128,g.lang==LANG_EN?L"DAY %d / %d":L"%d일차 / 총 %d일",g.day,g.totalDays);
    textW(dc,665,12,buf,RGB(34,55,95),fontSmall);

    moneyW(buf,128,p->cashPence);
    textW(dc,835,12,buf,RGB(24,132,95),fontSmall);

    // Line 2: Business Name & Debt (y = 34)
    textW(dc,665,34,p->businessName,stallColours[p->stallColour],fontSmall);

    if(p->debtPence > 0){
        wchar_t debtBuf[64];
        moneyW(debtBuf,64,p->debtPence);
        if(g.lang==LANG_EN) swprintf(buf,128,L"Debt: %s",debtBuf);
        else swprintf(buf,128,L"대출금: %s",debtBuf);
        textW(dc,815,34,buf,RGB(190,35,73),fontTiny);
    }

    // Line 3: Location & Beach Zone (y = 58)
    const wchar_t *zName = g.lang==LANG_EN ? beachZoneNamesEN[g.currentPlayer] : beachZoneNamesKO[g.currentPlayer];
    const wchar_t *locName = g.lang==LANG_EN ? locationsData[g.loc].nameEN : locationsData[g.loc].nameKO;
    swprintf(buf,128,L"%s (%s)",locName,zName);
    textW(dc,665,58,buf,RGB(90,105,125),fontTiny);

    // Line 4: Weather & Crowd (y = 78)
    const wchar_t *wName = g.lang==LANG_EN ? weatherNamesEN[g.weather] : weatherNamesKO[g.weather];
    if(g.lang==LANG_EN) swprintf(buf,128,L"%s  •  Crowd %d",wName,g.crowd);
    else swprintf(buf,128,L"%s  •  유동인구 %d명",wName,g.crowd);
    textW(dc,665,78,buf,RGB(34,55,95),fontSmall);

    // Line 5: Cost, Suggested & Bonus (y = 100)
    if(g.lang==LANG_EN) swprintf(buf,128,L"Cost %dp  Suggested %dp  Bonus +%d",g.unitCostPence,g.suggestedPricePence,p->bonusStock);
    else swprintf(buf,128,L"원가 %dp  권장가 %dp  보너스 +%d",g.unitCostPence,g.suggestedPricePence,p->bonusStock);
    textW(dc,665,100,buf,RGB(110,125,145),fontTiny);

    wchar_t v1[32],v2[32],v3[32];
    swprintf(v1,32,L"%d (+%d)",p->stock+p->bonusStock,p->bonusStock);
    moneyW(v2,32,p->pricePence);
    swprintf(v3,32,L"%d",p->adverts);
    
    sliders[0].value=p->stock;
    sliders[1].value=p->pricePence;
    sliders[2].value=p->adverts;

    drawSliderW(dc,sliders[0],g.lang==LANG_EN?L"Stock to make":L"제조할 재고량",v1);
    drawSliderW(dc,sliders[1],g.lang==LANG_EN?L"Price per candyfloss":L"개당 판매 가격",v2);
    drawSliderW(dc,sliders[2],g.lang==LANG_EN?L"Advertising signs":L"홍보 표지판 개수",v3);

    // Bank Loans Buttons
    planButtons[1].text = g.lang==LANG_EN ? L"BORROW £50" : L"£50 대출";
    planButtons[2].text = g.lang==LANG_EN ? L"REPAY £50" : L"£50 상환";
    drawButtonW(dc,planButtons[1],RGB(235,160,30));
    drawButtonW(dc,planButtons[2],p->debtPence>0&&p->cashPence>=5000?RGB(40,160,235):RGB(180,180,190));

    moneyW(buf,128,upfrontCost());
    textW(dc,685,470,g.lang==LANG_EN?L"Up-front cost":L"사전 준비 비용",RGB(62,74,95),fontSmall);
    textW(dc,835,470,buf,upfrontCost()>p->cashPence?RGB(190,35,73):RGB(30,130,92),fontSmall);

    planButtons[0].text = g.lang==LANG_EN ? L"OPEN FOR BUSINESS" : L"영업 개시";
    drawButtonW(dc,planButtons[0],upfrontCost()<=p->cashPence?RGB(244,58,139):RGB(180,180,190));

    RECT badge={15,15,145,52};
    fillRound(dc,badge,16,RGB(255,255,255));
    wchar_t wIconBuf[16];
    swprintf(wIconBuf,16,L"%S",weatherIcons[g.weather]);
    centreTextW(dc,badge,wIconBuf,RGB(30,55,95),fontSmall);
}

static void runDay(void) {
    PlayerState *p=&g.players[g.currentPlayer];
    g.eventId=irand(0,6);
    float eventDemand=1.0f; int totalStock=p->stock + p->bonusStock; int usable=totalStock; int effectiveCost=g.unitCostPence; int rep=0;
    
    p->totalStockMade += totalStock;

    if(g.eventId==1)eventDemand=1.28f;
    else if(g.eventId==2)usable=(int)(usable*.82f);
    else if(g.eventId==3)eventDemand=.78f;
    else if(g.eventId==4)eventDemand=1.35f;
    else if(g.eventId==5)effectiveCost=(int)(effectiveCost*1.16f);
    else if(g.eventId==6){eventDemand=1.14f;rep=4;}

    float ratio=(float)p->pricePence/g.suggestedPricePence, priceEffect;
    if(ratio<=.72f)priceEffect=1.28f;
    else if(ratio<=.92f)priceEffect=1.13f;
    else if(ratio<=1.08f)priceEffect=1.0f;
    else if(ratio<=1.30f)priceEffect=.78f;
    else if(ratio<=1.60f)priceEffect=.48f;
    else priceEffect=.18f;

    float adEffect=1.0f+(p->adverts*.035f>0.5f?0.5f:p->adverts*.035f);
    float repEffect=.72f+p->reputation/180.0f;
    float random=irand(88,112)/100.0f;
    float diff=g.difficulty==0?1.08f:g.difficulty==2?.93f:1.0f;

    p->customers=(int)(g.crowd*weatherDemand[g.weather]*priceEffect*adEffect*repEffect*random*eventDemand*diff+.5f);
    if(p->customers<0)p->customers=0;

    p->sold=p->customers<usable?p->customers:usable;
    p->unsold=totalStock-p->sold;
    p->revenuePence=p->sold*p->pricePence;
    p->costPence=p->stock*effectiveCost+p->adverts*150;
    p->profitPence=p->revenuePence-p->costPence;
    p->cashPence+=p->profitPence;

    // Apply daily bank loan interest (5%)
    if(p->debtPence > 0){
        p->debtPence = (int)(p->debtPence * 1.05f);
    }

    if(p->sold==0)rep-=4;
    else{
        if(ratio<=1.05f)rep+=1;
        if(ratio>1.35f)rep-=3;
        if(p->sold>=usable&&p->customers>usable)rep-=1;
        if(p->unsold>totalStock*.45f)rep-=1;
        if(p->adverts>=8)rep+=1;
    }
    p->reputation=clampi(p->reputation+rep,5,100);
    p->totalSold+=p->sold;
    if(p->profitPence>p->bestProfit)p->bestProfit=p->profitPence;

    g.screen=SCREEN_RESULTS;
    playSound(p->profitPence>=0 ? SOUND_PROFIT : SOUND_LOSS);
}

static void drawResults(HDC dc) {
    PlayerState *p=&g.players[g.currentPlayer];
    RECT all={0,0,960,600};HBRUSH b=CreateSolidBrush(RGB(92,72,155));FillRect(dc,&all,b);DeleteObject(b);
    RECT panel={100,40,860,570};fillRound(dc,panel,32,RGB(255,255,255));

    wchar_t buf[128];
    if(g.numPlayers>1){
        const wchar_t *pName = g.lang==LANG_EN ? playerNamesEN[g.currentPlayer] : playerNamesKO[g.currentPlayer];
        if(g.lang==LANG_EN) swprintf(buf,128,L"%s (%s) - DAY %d RESULTS",pName,p->businessName,g.day);
        else swprintf(buf,128,L"%s (%s) - %d일차 결과",pName,p->businessName,g.day);
        centreTextW(dc,(RECT){150,55,810,95},buf,stallColours[p->stallColour],fontMedium);
    }

    const wchar_t *eName = g.lang==LANG_EN ? eventNamesEN[g.eventId] : eventNamesKO[g.eventId];
    const wchar_t *eText = g.lang==LANG_EN ? eventTextsEN[g.eventId] : eventTextsKO[g.eventId];

    centreTextW(dc,(RECT){150,85,810,130},eName,RGB(36,56,95),fontLarge);
    wrappedTextW(dc,(RECT){170,130,790,170},eText,RGB(91,102,120),fontSmall,DT_CENTER);

    moneyW(buf,128,p->profitPence);
    centreTextW(dc,(RECT){280,175,680,235},buf,p->profitPence>=0?RGB(23,139,99):RGB(188,37,77),fontTitle);

    const wchar_t *labsEN[]={L"Customers",L"Sold",L"Unsold",L"Revenue",L"Costs",L"Cash total"};
    const wchar_t *labsKO[]={L"방문 손님",L"판매 수량",L"남은 재고",L"총 매출",L"총 비용",L"보유 자산"};
    int vals[]={p->customers,p->sold,p->unsold,p->revenuePence,p->costPence,p->cashPence};
    for(int i=0;i<6;i++){
        int col=i%3,row=i/3;
        RECT rr={160+col*220,250+row*100,350+col*220,335+row*100};
        fillRound(dc,rr,18,RGB(240,248,252));
        centreTextW(dc,(RECT){rr.left,rr.top+6,rr.right,rr.top+30},g.lang==LANG_EN?labsEN[i]:labsKO[i],RGB(96,108,126),fontTiny);
        if(i>=3)moneyW(buf,128,vals[i]);
        else swprintf(buf,128,L"%d",vals[i]);
        centreTextW(dc,(RECT){rr.left,rr.top+30,rr.right,rr.bottom-4},buf,RGB(34,55,95),fontMedium);
    }

    static wchar_t btnBuf[128];
    if(g.currentPlayer<g.numPlayers-1){
        const wchar_t *nextPName = g.lang==LANG_EN ? playerNamesEN[g.currentPlayer+1] : playerNamesKO[g.currentPlayer+1];
        if(g.lang==LANG_EN) swprintf(btnBuf,128,L"NEXT PLAYER (%s)",nextPName);
        else swprintf(btnBuf,128,L"다음 플레이어 (%s)",nextPName);
        resultButtons[0].text=btnBuf;
    } else if(g.day>=g.totalDays){
        resultButtons[0].text=g.lang==LANG_EN ? L"SEE FINAL LEADERBOARD" : L"최종 순위표 보기";
    } else {
        resultButtons[0].text=g.lang==LANG_EN ? L"NEXT DAY" : L"다음 날로";
    }
    drawButtonW(dc,resultButtons[0],RGB(244,58,139));
}

static void drawFinal(HDC dc) {
    RECT all={0,0,960,600};HBRUSH b=CreateSolidBrush(RGB(250,139,190));FillRect(dc,&all,b);DeleteObject(b);
    RECT p={100,45,860,565};fillRound(dc,p,32,RGB(255,255,255));

    if(g.numPlayers==1){
        PlayerState *p0=&g.players[0];
        int netWealth = p0->cashPence - p0->debtPence;
        int score = netWealth + p0->reputation*45 + p0->totalSold*5;

        const wchar_t *titleStr;
        if(netWealth<0) titleStr = g.lang==LANG_EN ? L"STALL CLOSED (BANKRUPT)" : L"영업 종료 (파산)";
        else if(score>=12000) titleStr = g.lang==LANG_EN ? L"CANDYFLOSS CHAMPION!" : L"솜사탕 챔피언!";
        else titleStr = g.lang==LANG_EN ? L"SEASON COMPLETE" : L"시즌 완수!";
        
        centreTextW(dc,(RECT){150,65,810,120},titleStr,RGB(37,58,99),fontLarge);
        
        wchar_t buf[128];
        moneyW(buf,128,netWealth);
        centreTextW(dc,(RECT){210,175,460,235},buf,RGB(25,139,98),fontLarge);
        centreTextW(dc,(RECT){210,150,460,175},g.lang==LANG_EN?L"NET WEALTH":L"순 자산",RGB(101,110,127),fontTiny);

        swprintf(buf,128,L"%d",p0->reputation);
        centreTextW(dc,(RECT){500,175,750,235},buf,RGB(40,161,229),fontLarge);
        centreTextW(dc,(RECT){500,150,750,175},g.lang==LANG_EN?L"REPUTATION":L"평판",RGB(101,110,127),fontTiny);

        swprintf(buf,128,L"%d",p0->totalSold);
        centreTextW(dc,(RECT){210,310,460,370},buf,RGB(235,71,139),fontLarge);
        centreTextW(dc,(RECT){210,285,460,310},g.lang==LANG_EN?L"TOTAL SOLD":L"총 판매량",RGB(101,110,127),fontTiny);

        moneyW(buf,128,p0->bestProfit);
        centreTextW(dc,(RECT){500,310,750,370},buf,RGB(235,154,42),fontLarge);
        centreTextW(dc,(RECT){500,285,750,310},g.lang==LANG_EN?L"BEST PROFIT":L"최대 일일이익",RGB(101,110,127),fontTiny);

        checkAndAddHighScore(playerNamesEN[0], p0->businessName, locationsData[g.loc].nameEN, score, netWealth, p0->reputation, p0->totalSold);
    } else {
        centreTextW(dc,(RECT){150,55,810,105},g.lang==LANG_EN?L"FINAL LEADERBOARD":L"최종 순위표",RGB(37,58,99),fontLarge);

        int order[4]={0,1,2,3};
        int scores[4];
        for(int i=0;i<g.numPlayers;i++){
            int netW = g.players[i].cashPence - g.players[i].debtPence;
            scores[i]=netW + g.players[i].reputation*45 + g.players[i].totalSold*5;
        }
        for(int i=0;i<g.numPlayers-1;i++){
            for(int j=i+1;j<g.numPlayers;j++){
                if(scores[order[j]] > scores[order[i]]){
                    int tmp=order[i]; order[i]=order[j]; order[j]=tmp;
                }
            }
        }

        const wchar_t *ranksEN[]={L"1st WINNER",L"2nd PLACE",L"3rd PLACE",L"4th PLACE"};
        const wchar_t *ranksKO[]={L"1위 (우승)",L"2위",L"3위",L"4위"};
        COLORREF rankCols[]={RGB(235,170,30), RGB(130,140,155), RGB(185,110,50), RGB(110,120,135)};

        for(int i=0;i<g.numPlayers;i++){
            int pIdx=order[i];
            PlayerState *pl=&g.players[pIdx];
            RECT rowR={140,120+i*80,820,185+i*80};
            fillRound(dc,rowR,18,i==0?RGB(255,248,220):RGB(242,247,252));

            RECT rankR={155,120+i*80,260,185+i*80};
            centreTextW(dc,rankR,g.lang==LANG_EN?ranksEN[i]:ranksKO[i],rankCols[i],fontSmall);
            const wchar_t *pName = g.lang==LANG_EN ? playerNamesEN[pIdx] : playerNamesKO[pIdx];
            wchar_t bTag[64];
            swprintf(bTag,64,L"%s (%s)",pName,pl->businessName);
            textW(dc,270,130+i*80,bTag,stallColours[pl->stallColour],fontSmall);

            wchar_t buf[64];
            int netW = pl->cashPence - pl->debtPence;
            moneyW(buf,64,netW);
            RECT cashR={440,120+i*80,560,185+i*80};
            centreTextW(dc,cashR,buf,RGB(25,139,98),fontSmall);

            if(g.lang==LANG_EN) swprintf(buf,64,L"Rep: %d",pl->reputation);
            else swprintf(buf,64,L"평판: %d",pl->reputation);
            RECT repR={570,120+i*80,670,185+i*80};
            centreTextW(dc,repR,buf,RGB(40,150,220),fontSmall);

            if(g.lang==LANG_EN) swprintf(buf,64,L"Sold: %d",pl->totalSold);
            else swprintf(buf,64,L"판매: %d",pl->totalSold);
            RECT soldR={680,120+i*80,800,185+i*80};
            centreTextW(dc,soldR,buf,RGB(235,71,139),fontSmall);

            if(i==0){
                checkAndAddHighScore(pName, pl->businessName, locationsData[g.loc].nameEN, scores[pIdx], netW, pl->reputation, pl->totalSold);
            }
        }
    }

    finalButtons[0].text = g.lang==LANG_EN ? L"PLAY AGAIN" : L"다시 시작";
    finalButtons[1].text = g.lang==LANG_EN ? L"PERFORMANCE REPORT" : L"성과 분석 보고서";

    drawButtonW(dc,finalButtons[0],RGB(244,58,139));
    drawButtonW(dc,finalButtons[1],RGB(40,160,235));
}

static void paint(HDC dc) {
    if(!backDC){
        backDC=CreateCompatibleDC(dc);
        backBmp=CreateCompatibleBitmap(dc,APP_W,APP_H);
        SelectObject(backDC,backBmp);
    }
    RECT all={0,0,APP_W,APP_H};
    FillRect(backDC,&all,(HBRUSH)GetStockObject(WHITE_BRUSH));

    switch(g.screen){
        case SCREEN_TITLE:drawTitle(backDC);break;
        case SCREEN_SETUP:drawSetup(backDC);break;
        case SCREEN_PASS_TURN:drawPassTurn(backDC);break;
        case SCREEN_SPIN:drawSpinningMiniGame(backDC);break;
        case SCREEN_PLAN:drawPlan(backDC);break;
        case SCREEN_RESULTS:drawResults(backDC);break;
        case SCREEN_FINAL:drawFinal(backDC);break;
        case SCREEN_HOF:drawHallOfFame(backDC);break;
        case SCREEN_ANALYSIS:drawPerformanceAnalysis(backDC);break;
    }
    StretchBlt(dc,0,0,clientW,clientH,backDC,0,0,APP_W,APP_H,SRCCOPY);
}

static void setSliderFromMouse(int index,int x) {
    Slider *s=&sliders[index];
    x=clampi(x,s->r.left,s->r.right);
    s->value=s->minv+(int)((double)(x-s->r.left)/(s->r.right-s->r.left)*(s->maxv-s->minv));
    PlayerState *p=&g.players[g.currentPlayer];
    if(s->id==1){s->value=(s->value/5)*5;p->stock=s->value;}
    else if(s->id==2){s->value=(s->value/10)*10;p->pricePence=s->value;}
    else{p->adverts=s->value;}
}

static void clickAt(int x,int y) {
    if(g.screen==SCREEN_TITLE){
        if(ptIn(titleButtons[0].r,x,y)){ playSound(SOUND_CLICK); g.screen=SCREEN_SETUP; }
        else if(ptIn(titleButtons[1].r,x,y)){
            playSound(SOUND_CLICK);
            const wchar_t *msg = g.lang==LANG_EN ?
                L"Choose stock, set a price, buy advertising and manage bank loans. Spin the mouse in circles during the mini-game to earn bonus candyfloss! Hotseat multiplayer allows 1 to 4 players to compete across UK & Korean beaches!" :
                L"재고를 준비하고, 가격을 설정하고, 광고를 집행하고 대출을 관리하세요. 미니게임에서 마우스를 원형으로 돌려 보너스 솜사탕을 얻으세요! 영국 및 한국의 유명 해변에서 1~4인이 대결할 수 있습니다!";
            const wchar_t *head = g.lang==LANG_EN ? L"How to play" : L"게임 설명";
            MessageBoxW(NULL,msg,head,MB_OK|MB_ICONINFORMATION);
        }
        else if(ptIn(titleButtons[2].r,x,y)){ playSound(SOUND_CLICK); g.screen=SCREEN_HOF; }
        else if(ptIn(titleButtons[3].r,x,y)){ g.soundOn=!g.soundOn; playSound(SOUND_CLICK); }
        else if(ptIn(titleButtons[4].r,x,y)){
            g.lang=(g.lang==LANG_EN?LANG_KO:LANG_EN);
            playSound(SOUND_CLICK);
            for(int i=0;i<4;i++){
                for(int k=0;k<6;k++){
                    if(wcscmp(g.players[i].businessName, defaultBizNamesEN[k])==0 || wcscmp(g.players[i].businessName, defaultBizNamesKO[k])==0){
                        wcsncpy(g.players[i].businessName, g.lang==LANG_EN ? defaultBizNamesEN[k] : defaultBizNamesKO[k], 31);
                        break;
                    }
                }
            }
        }
    }
    else if(g.screen==SCREEN_SETUP){
        if(ptIn(setupButtons[1].r,x,y)){ playSound(SOUND_CLICK); g.screen=SCREEN_TITLE; return; }
        if(ptIn(setupButtons[0].r,x,y)){
            playSound(SOUND_COIN);
            g.day=1;
            for(int i=0;i<4;i++){
                g.players[i].active=(i<g.numPlayers);
                g.players[i].cashPence=g.difficulty==0?4500:g.difficulty==2?3000:3500;
                g.players[i].debtPence=0;
                g.players[i].reputation=50;
                g.players[i].totalSold=0;
                g.players[i].totalStockMade=0;
                g.players[i].bestProfit=-999999;
                if(g.numPlayers>1) g.players[i].stallColour=i;
            }
            prepareDay();
            if(g.numPlayers>1) g.screen=SCREEN_PASS_TURN;
            else {
                g.spinStartTick = GetTickCount();
                g.spinTotalAngle = 0;
                g.bonusEarned = 0;
                g.screen = SCREEN_SPIN;
            }
            return;
        }

        // Location click
        for(int i=0;i<7;i++){
            RECT r={60+i*122,105,175+i*122,145};
            if(ptIn(r,x,y)){ playSound(SOUND_CLICK); g.loc=(Location)i; }
        }

        for(int i=0;i<4;i++){
            RECT r={60+i*140,195,185+i*140,235};
            if(ptIn(r,x,y)){ playSound(SOUND_CLICK); g.numPlayers=i+1; }
        }
        for(int i=0;i<3;i++){
            RECT r={630+i*100,195,720+i*100,235};
            if(ptIn(r,x,y)){ playSound(SOUND_CLICK); g.totalDays=i==0?7:i==1?10:14; }
        }

        // Business Name Box Click for Typing
        for(int i=0;i<g.numPlayers;i++){
            RECT r={60,285+i*52,500,325+i*52};
            if(ptIn(r,x,y)){ playSound(SOUND_CLICK); g.editingNamePlayer=i; }

            // Preset button click
            RECT pr={515,285+i*52,615,325+i*52};
            if(ptIn(pr,x,y)){
                playSound(SOUND_CLICK);
                int rIdx = irand(0,5);
                wcsncpy(g.players[i].businessName, g.lang==LANG_EN?defaultBizNamesEN[rIdx]:defaultBizNamesKO[rIdx], 31);
            }
        }

        if(g.numPlayers==1){
            for(int i=0;i<4;i++){
                RECT r={640+i*70,285,700+i*70,325};
                if(ptIn(r,x,y)){ playSound(SOUND_CLICK); g.players[0].stallColour=i; }
            }
        }
    }
    else if(g.screen==SCREEN_PASS_TURN){
        if(ptIn(passButtons[0].r,x,y)){
            playSound(SOUND_CLICK);
            g.spinStartTick = GetTickCount();
            g.spinTotalAngle = 0;
            g.bonusEarned = 0;
            g.screen = SCREEN_SPIN;
        }
    }
    else if(g.screen==SCREEN_PLAN){
        for(int i=0;i<3;i++)if(ptIn(sliders[i].r,x,y)){g.activeSlider=i;setSliderFromMouse(i,x);}
        
        // Borrow £50 Loan
        if(ptIn(planButtons[1].r,x,y)){
            playSound(SOUND_COIN);
            g.players[g.currentPlayer].cashPence += 5000;
            g.players[g.currentPlayer].debtPence += 5250; // includes £2.50 upfront fee
        }
        // Repay £50 Loan
        if(ptIn(planButtons[2].r,x,y) && g.players[g.currentPlayer].debtPence>0 && g.players[g.currentPlayer].cashPence>=5000){
            playSound(SOUND_COIN);
            g.players[g.currentPlayer].cashPence -= 5000;
            g.players[g.currentPlayer].debtPence -= 5000;
            if(g.players[g.currentPlayer].debtPence < 0) g.players[g.currentPlayer].debtPence = 0;
        }

        if(ptIn(planButtons[0].r,x,y)&&upfrontCost()<=g.players[g.currentPlayer].cashPence){
            playSound(SOUND_COIN);
            runDay();
        }
    }
    else if(g.screen==SCREEN_RESULTS&&ptIn(resultButtons[0].r,x,y)){
        playSound(SOUND_CLICK);
        if(g.currentPlayer<g.numPlayers-1){
            g.currentPlayer++;
            if(g.numPlayers>1) g.screen=SCREEN_PASS_TURN;
            else {
                g.spinStartTick = GetTickCount();
                g.spinTotalAngle = 0;
                g.bonusEarned = 0;
                g.screen = SCREEN_SPIN;
            }
        } else {
            if(g.day>=g.totalDays){
                playSound(SOUND_WIN);
                g.screen=SCREEN_FINAL;
            } else {
                g.day++;
                prepareDay();
                if(g.numPlayers>1) g.screen=SCREEN_PASS_TURN;
                else {
                    g.spinStartTick = GetTickCount();
                    g.spinTotalAngle = 0;
                    g.bonusEarned = 0;
                    g.screen = SCREEN_SPIN;
                }
            }
        }
    }
    else if(g.screen==SCREEN_FINAL){
        if(ptIn(finalButtons[0].r,x,y)){ playSound(SOUND_CLICK); g.screen=SCREEN_SETUP; }
        else if(ptIn(finalButtons[1].r,x,y)){ playSound(SOUND_CLICK); g.screen=SCREEN_ANALYSIS; }
    }
    else if(g.screen==SCREEN_HOF&&ptIn(hofButtons[0].r,x,y)){
        playSound(SOUND_CLICK);
        g.screen=SCREEN_TITLE;
    }
    else if(g.screen==SCREEN_ANALYSIS&&ptIn(analysisButtons[0].r,x,y)){
        playSound(SOUND_CLICK);
        g.screen=SCREEN_FINAL;
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    switch(msg){
        case WM_CREATE:SetTimer(hwnd,TIMER_ID,33,NULL);return 0;
        case WM_SIZE:clientW=LOWORD(lp);clientH=HIWORD(lp);return 0;
        case WM_TIMER:
            g.tick++;
            if(g.screen==SCREEN_SPIN){
                POINT pt; GetCursorPos(&pt); ScreenToClient(hwnd, &pt);
                int mx = pt.x * APP_W / (clientW?clientW:1);
                int my = pt.y * APP_H / (clientH?clientH:1);
                double angle = atan2((double)(my - 290), (double)(mx - 480));
                double diff = angle - g.lastMouseAngle;
                if(diff > PI) diff -= 2*PI;
                if(diff < -PI) diff += 2*PI;
                g.lastMouseAngle = angle;
                g.spinTotalAngle += fabs(diff);
                int newBonus = (int)(g.spinTotalAngle / (PI * 1.8));
                if(newBonus > 25) newBonus = 25;
                if(newBonus > g.bonusEarned){
                    g.bonusEarned = newBonus;
                    playSound(SOUND_CLICK);
                }
            }
            InvalidateRect(hwnd,NULL,FALSE);
            return 0;
        case WM_CHAR:
            if(g.screen==SCREEN_SETUP && g.editingNamePlayer>=0 && g.editingNamePlayer<g.numPlayers){
                wchar_t *name = g.players[g.editingNamePlayer].businessName;
                size_t len = wcslen(name);
                if(wp==VK_BACK){
                    if(len>0) name[len-1] = L'\0';
                } else if(wp==VK_RETURN){
                    g.editingNamePlayer = -1;
                } else if(wp>=32 && len<22){
                    name[len] = (wchar_t)wp;
                    name[len+1] = L'\0';
                }
                InvalidateRect(hwnd,NULL,FALSE);
                return 0;
            }
            return 0;
        case WM_LBUTTONDOWN:{
            SetCapture(hwnd);g.mouseDown=1;
            int x=LOWORD(lp)*APP_W/(clientW?clientW:1),y=HIWORD(lp)*APP_H/(clientH?clientH:1);
            clickAt(x,y);InvalidateRect(hwnd,NULL,FALSE);return 0;
        }
        case WM_MOUSEMOVE:
            if(g.mouseDown&&g.screen==SCREEN_PLAN&&g.activeSlider>=0){
                int x=LOWORD(lp)*APP_W/(clientW?clientW:1);
                setSliderFromMouse(g.activeSlider,x);
                InvalidateRect(hwnd,NULL,FALSE);
            }
            return 0;
        case WM_LBUTTONUP:g.mouseDown=0;g.activeSlider=-1;ReleaseCapture();return 0;
        case WM_KEYDOWN:
            if(wp==VK_ESCAPE){
                if(g.screen==SCREEN_TITLE)PostQuitMessage(0);
                else g.screen=SCREEN_TITLE;
                InvalidateRect(hwnd,NULL,FALSE);
            }
            return 0;
        case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);paint(dc);EndPaint(hwnd,&ps);return 0;}
        case WM_DESTROY:KillTimer(hwnd,TIMER_ID);if(backDC)DeleteDC(backDC);if(backBmp)DeleteObject(backBmp);PostQuitMessage(0);return 0;
    }return DefWindowProc(hwnd,msg,wp,lp);
}

int APIENTRY WinMain(HINSTANCE h,HINSTANCE prev,LPSTR cmd,int show){
    (void)prev;(void)cmd;srand((unsigned)time(NULL));ZeroMemory(&g,sizeof(g));
    g.screen=SCREEN_TITLE;g.lang=LANG_KO;g.loc=LOC_BLACKPOOL;g.numPlayers=1;g.totalDays=10;g.difficulty=1;g.soundOn=1;g.activeSlider=-1;g.editingNamePlayer=-1;
    
    loadHighScores();

    for(int i=0;i<4;i++){
        wcsncpy(g.players[i].businessName, defaultBizNamesKO[i], 31);
        g.players[i].stallColour=i;
    }
    
    fontTitle=CreateFontW(54,0,0,0,FW_BLACK,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Malgun Gothic");
    fontLarge=CreateFontW(32,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Malgun Gothic");
    fontMedium=CreateFontW(22,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Malgun Gothic");
    fontSmall=CreateFontW(17,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Malgun Gothic");
    fontTiny=CreateFontW(14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Malgun Gothic");

    WNDCLASSA wc={0};wc.lpfnWndProc=WndProc;wc.hInstance=h;wc.hCursor=LoadCursor(NULL,IDC_ARROW);wc.hIcon=LoadIcon(h,MAKEINTRESOURCE(1));wc.lpszClassName="CandyflossFloppy";wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassA(&wc);
    RECT r={0,0,APP_W,APP_H};AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,FALSE);HWND hwnd=CreateWindowA(wc.lpszClassName,"Candyfloss Tycoon - Floppy Edition",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,NULL,NULL,h,NULL);if(!hwnd)return 0;ShowWindow(hwnd,show);UpdateWindow(hwnd);
    MSG m;while(GetMessage(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessage(&m);}
    DeleteObject(fontTitle);DeleteObject(fontLarge);DeleteObject(fontMedium);DeleteObject(fontSmall);DeleteObject(fontTiny);return (int)m.wParam;
}
