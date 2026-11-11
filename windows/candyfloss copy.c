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

typedef enum { SCREEN_TITLE, SCREEN_SETUP, SCREEN_PASS_TURN, SCREEN_PLAN, SCREEN_RESULTS, SCREEN_FINAL } Screen;
typedef enum { WEATHER_SUNNY, WEATHER_BREEZY, WEATHER_CLOUDY, WEATHER_RAIN, WEATHER_HEAT, WEATHER_STORM } Weather;
typedef enum { SOUND_CLICK, SOUND_COIN, SOUND_PROFIT, SOUND_LOSS, SOUND_WIN } SoundType;
typedef enum { LANG_EN, LANG_KO } Language;

typedef struct { RECT r; const wchar_t *text; int id; } UiButtonW;
typedef struct { RECT r; int minv, maxv, value; int id; } Slider;

typedef struct {
    int active;
    int cashPence;
    int reputation;
    int stock, pricePence, adverts;
    int sold, customers, unsold;
    int revenuePence, costPence, profitPence;
    int totalSold, bestProfit;
    int stallColour;
} PlayerState;

typedef struct {
    Screen screen;
    Language lang;
    int numPlayers;
    int currentPlayer;
    int day, totalDays;
    int difficulty;
    int soundOn;
    int unitCostPence, suggestedPricePence, crowd;
    Weather weather;
    int eventId;
    PlayerState players[4];
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
    r=(RECT){480,180,585,230}; b=CreateSolidBrush(RGB(250,239,244));FillRect(dc,&r,b);DeleteObject(b);
    r=(RECT){470,169,595,184}; b=CreateSolidBrush(RGB(235,74,139));FillRect(dc,&r,b);DeleteObject(b);
    textW(dc,511,194,L"PIER",RGB(35,59,98),fontMedium);
    // stall
    COLORREF sc=stallColours[g.players[g.currentPlayer].stallColour];
    r=(RECT){45,235,330,465}; b=CreateSolidBrush(lighten(sc,130));FillRect(dc,&r,b);DeleteObject(b);
    for(int i=0;i<6;i++){ RECT aw={35+i*52,205,87+i*52,248}; b=CreateSolidBrush(i%2?RGB(255,247,239):sc);FillRect(dc,&aw,b);DeleteObject(b); }
    r=(RECT){35,390,350,415}; b=CreateSolidBrush(RGB(126,79,50));FillRect(dc,&r,b);DeleteObject(b);
    r=(RECT){90,145,300,205}; fillRound(dc,r,18,RGB(255,255,255));
    centreTextW(dc,(RECT){95,150,295,178},g.lang==LANG_EN?L"CANDYFLOSS":L"솜사탕",sc,fontLarge);
    centreTextW(dc,(RECT){95,176,295,198},g.lang==LANG_EN?L"TYCOON":L"타이쿤",RGB(39,63,104),fontSmall);
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
    {{360,230,600,280},L"PLAY GAME",1},
    {{360,292,600,342},L"HOW TO PLAY",2},
    {{360,354,600,404},L"SOUND: ON",3},
    {{360,416,600,466},L"LANG: EN",4}
};
static UiButtonW setupButtons[] = {
    {{330,505,630,565},L"START GAME",10}, {{30,30,140,68},L"BACK",11}
};
static UiButtonW passButtons[] = {
    {{340,420,620,480},L"READY TO PLAN",15}
};
static UiButtonW planButtons[] = { {{700,505,920,565},L"OPEN FOR BUSINESS",20} };
static UiButtonW resultButtons[] = { {{340,510,620,565},L"NEXT PLAYER",30} };
static UiButtonW finalButtons[] = { {{360,515,600,570},L"PLAY AGAIN",40} };

static Slider sliders[] = {
    {{685,245,915,275},10,250,80,1},
    {{685,335,915,365},100,500,250,2},
    {{685,425,915,455},0,20,3,3}
};

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

    centreTextW(dc,(RECT){133,38,833,118},t1,RGB(120,20,65),fontTitle);
    centreTextW(dc,(RECT){130,35,830,115},t1,RGB(255,255,255),fontTitle);

    centreTextW(dc,(RECT){253,113,713,173},t2,RGB(160,110,0),fontLarge);
    centreTextW(dc,(RECT){250,110,710,170},t2,RGB(255,215,0),fontLarge);

    RECT badge={280,175,680,215};
    fillRound(dc,badge,20,RGB(34,55,95));
    centreTextW(dc,badge,sub,RGB(255,235,130),fontSmall);

    titleButtons[0].text = g.lang==LANG_EN ? L"PLAY GAME" : L"게임 시작";
    titleButtons[1].text = g.lang==LANG_EN ? L"HOW TO PLAY" : L"게임 설명";
    titleButtons[2].text = g.soundOn ? (g.lang==LANG_EN?L"SOUND: ON":L"소리: 켜짐") : (g.lang==LANG_EN?L"SOUND: OFF":L"소리: 꺼짐");
    titleButtons[3].text = g.lang==LANG_EN ? L"LANG: EN" : L"언어: 한국어";

    for(int i=0;i<4;i++){
        COLORREF btnCol=(i==0)?RGB(255,50,135):(i==1?RGB(40,160,235):(i==2?(g.soundOn?RGB(40,190,120):RGB(235,140,40)):RGB(140,90,210)));
        drawButtonW(dc,titleButtons[i],btnCol);
    }

    const wchar_t *footer = g.lang==LANG_EN ? L"Single-File Win32 C Engine • 0 External Dependencies • Floppy Disk Compatible" : L"단일 C 파일 Win32 엔진 • 외부 의존성 0% • 플로피 디스크 용량 지원";
    centreTextW(dc,(RECT){100,545,860,580},footer,RGB(90,105,125),fontTiny);
}

static void drawSetup(HDC dc) {
    RECT all={0,0,960,600}; HBRUSH b=CreateSolidBrush(RGB(222,247,255));FillRect(dc,&all,b);DeleteObject(b);
    centreTextW(dc,(RECT){160,25,800,85},g.lang==LANG_EN?L"GAME SETUP":L"게임 설정",RGB(30,55,96),fontLarge);
    
    textW(dc,210,105,g.lang==LANG_EN?L"Number of players":L"플레이어 수",RGB(35,55,90),fontMedium);
    const wchar_t *pCountEN[] = {L"1 PLAYER", L"2 PLAYERS", L"3 PLAYERS", L"4 PLAYERS"};
    const wchar_t *pCountKO[] = {L"1인용", L"2인용", L"3인용", L"4인용"};
    for(int i=0;i<4;i++){
        RECT r={180+i*140,140,305+i*140,185};
        fillRound(dc,r,16,g.numPlayers==(i+1)?RGB(255,75,151):RGB(190,219,235));
        centreTextW(dc,r,g.lang==LANG_EN?pCountEN[i]:pCountKO[i],RGB(255,255,255),fontTiny);
    }

    textW(dc,210,205,g.lang==LANG_EN?L"Game length":L"게임 기간",RGB(35,55,90),fontMedium);
    const wchar_t *daysEN[] = {L"7 DAYS",L"10 DAYS",L"14 DAYS"};
    const wchar_t *daysKO[] = {L"7일간",L"10일간",L"14일간"};
    for(int i=0;i<3;i++){
        RECT r={205+i*190,240,355+i*190,285};
        fillRound(dc,r,18,g.totalDays==(i==0?7:i==1?10:14)?RGB(255,75,151):RGB(190,219,235));
        centreTextW(dc,r,g.lang==LANG_EN?daysEN[i]:daysKO[i],RGB(255,255,255),fontSmall);
    }

    textW(dc,210,305,g.lang==LANG_EN?L"Difficulty":L"난이도",RGB(35,55,90),fontMedium);
    const wchar_t *diffEN[] = {L"EASY",L"NORMAL",L"HARD"};
    const wchar_t *diffKO[] = {L"쉬움",L"보통",L"어려움"};
    for(int i=0;i<3;i++){
        RECT r={205+i*190,340,355+i*190,385};
        fillRound(dc,r,18,g.difficulty==i?RGB(42,166,232):RGB(190,219,235));
        centreTextW(dc,r,g.lang==LANG_EN?diffEN[i]:diffKO[i],RGB(255,255,255),fontSmall);
    }

    if(g.numPlayers==1){
        textW(dc,210,405,g.lang==LANG_EN?L"Stall colour":L"점포 색상",RGB(35,55,90),fontMedium);
        for(int i=0;i<4;i++){
            RECT r={250+i*120,438,330+i*120,482};
            fillRound(dc,r,16,stallColours[i]);
            if(g.players[0].stallColour==i){ FrameRect(dc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH)); }
        }
    } else {
        const wchar_t *mpMsg = g.lang==LANG_EN ? L"Multiplayer stall colors: P1 Pink, P2 Blue, P3 Yellow, P4 Green" : L"다인용 점포 색상: P1 분홍, P2 파랑, P3 노랑, P4 초록";
        centreTextW(dc,(RECT){150,420,810,470},mpMsg,RGB(80,100,130),fontSmall);
    }

    setupButtons[0].text = g.lang==LANG_EN ? L"START GAME" : L"게임 시작";
    setupButtons[1].text = g.lang==LANG_EN ? L"BACK" : L"뒤로";

    drawButtonW(dc,setupButtons[0],RGB(244,58,139));
    drawButtonW(dc,setupButtons[1],RGB(43,163,232));
}

static void drawPassTurn(HDC dc) {
    RECT all={0,0,960,600}; HBRUSH b=CreateSolidBrush(RGB(42,163,232)); FillRect(dc,&all,b); DeleteObject(b);
    RECT p={150,90,810,510}; fillRound(dc,p,32,RGB(255,255,255));
    
    wchar_t buf[128];
    if(g.lang==LANG_EN) swprintf(buf,128,L"DAY %d / %d",g.day,g.totalDays);
    else swprintf(buf,128,L"%d일차 / 총 %d일",g.day,g.totalDays);
    centreTextW(dc,(RECT){200,120,760,165},buf,RGB(100,115,135),fontMedium);
    
    const wchar_t *pName = g.lang==LANG_EN ? playerNamesEN[g.currentPlayer] : playerNamesKO[g.currentPlayer];
    if(g.lang==LANG_EN) swprintf(buf,128,L"PASS THE MOUSE TO %s",pName);
    else swprintf(buf,128,L"%s 차례입니다 (마우스 전달)",pName);
    centreTextW(dc,(RECT){180,180,780,245},buf,stallColours[g.players[g.currentPlayer].stallColour],fontTitle);

    wchar_t mbuf[64];
    moneyW(mbuf,64,g.players[g.currentPlayer].cashPence);
    if(g.lang==LANG_EN) swprintf(buf,128,L"Current Balance: %s  |  Reputation: %d",mbuf,g.players[g.currentPlayer].reputation);
    else swprintf(buf,128,L"현재 잔액: %s  |  평판: %d점",mbuf,g.players[g.currentPlayer].reputation);
    centreTextW(dc,(RECT){180,265,780,315},buf,RGB(45,65,95),fontMedium);

    const wchar_t *wName = g.lang==LANG_EN ? weatherNamesEN[g.weather] : weatherNamesKO[g.weather];
    if(g.lang==LANG_EN) swprintf(buf,128,L"Weather Today: %s  (Crowd: %d)",wName,g.crowd);
    else swprintf(buf,128,L"오늘 날씨: %s  (유동인구: %d명)",wName,g.crowd);
    centreTextW(dc,(RECT){180,330,780,375},buf,RGB(90,105,125),fontSmall);

    passButtons[0].text = g.lang==LANG_EN ? L"READY TO PLAN" : L"준비 완료";
    drawButtonW(dc,passButtons[0],RGB(244,58,139));
}

static void prepareDay(void) {
    int roll=irand(1,100);
    g.weather=roll<=27?WEATHER_SUNNY:roll<=52?WEATHER_BREEZY:roll<=72?WEATHER_CLOUDY:roll<=87?WEATHER_RAIN:roll<=95?WEATHER_HEAT:WEATHER_STORM;
    g.crowd=irand(82,145)+(g.day-1)*3;
    g.unitCostPence=clampi(42+(g.day-1)*2+irand(-4,6),25,85);
    g.suggestedPricePence=235+(g.day-1)*5+irand(-12,12);
    g.currentPlayer=0;

    for(int i=0;i<g.numPlayers;i++){
        PlayerState *p=&g.players[i];
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
    if(g.lang==LANG_EN) swprintf(buf,128,L"DAY %d / %d",g.day,g.totalDays);
    else swprintf(buf,128,L"%d일차 / 총 %d일",g.day,g.totalDays);
    textW(dc,670,18,buf,RGB(34,55,95),fontSmall);
    
    if(g.numPlayers>1){
        const wchar_t *pName = g.lang==LANG_EN ? playerNamesEN[g.currentPlayer] : playerNamesKO[g.currentPlayer];
        textW(dc,670,42,pName,stallColours[p->stallColour],fontMedium);
    }
    
    moneyW(buf,128,p->cashPence);
    textW(dc,815,18,buf,RGB(24,132,95),fontSmall);

    const wchar_t *wName = g.lang==LANG_EN ? weatherNamesEN[g.weather] : weatherNamesKO[g.weather];
    if(g.lang==LANG_EN) swprintf(buf,128,L"%s - Crowd %d",wName,g.crowd);
    else swprintf(buf,128,L"%s - 유동인구 %d명",wName,g.crowd);
    textW(dc,670,78,buf,RGB(34,55,95),fontSmall);

    if(g.lang==LANG_EN) swprintf(buf,128,L"Cost %dp each  Suggested %dp",g.unitCostPence,g.suggestedPricePence);
    else swprintf(buf,128,L"원가 개당 %dp  권장가 %dp",g.unitCostPence,g.suggestedPricePence);
    textW(dc,670,108,buf,RGB(91,105,125),fontTiny);

    wchar_t v1[32],v2[32],v3[32];
    swprintf(v1,32,L"%d",p->stock);
    moneyW(v2,32,p->pricePence);
    swprintf(v3,32,L"%d",p->adverts);
    
    sliders[0].value=p->stock;
    sliders[1].value=p->pricePence;
    sliders[2].value=p->adverts;

    drawSliderW(dc,sliders[0],g.lang==LANG_EN?L"Stock to make":L"제조할 재고량",v1);
    drawSliderW(dc,sliders[1],g.lang==LANG_EN?L"Price per candyfloss":L"개당 판매 가격",v2);
    drawSliderW(dc,sliders[2],g.lang==LANG_EN?L"Advertising signs":L"홍보 표지판 개수",v3);

    moneyW(buf,128,upfrontCost());
    textW(dc,685,470,g.lang==LANG_EN?L"Up-front cost":L"사전 준비 비용",RGB(62,74,95),fontSmall);
    textW(dc,835,470,buf,upfrontCost()>p->cashPence?RGB(190,35,73):RGB(30,130,92),fontSmall);

    planButtons[0].text = g.lang==LANG_EN ? L"OPEN FOR BUSINESS" : L"영업 개시";
    drawButtonW(dc,planButtons[0],upfrontCost()<=p->cashPence?RGB(244,58,139):RGB(180,180,190));

    RECT badge={15,15,190,60};
    fillRound(dc,badge,18,RGB(255,255,255));
    wchar_t wIconBuf[16];
    swprintf(wIconBuf,16,L"%S",weatherIcons[g.weather]);
    centreTextW(dc,badge,wIconBuf,RGB(30,55,95),fontSmall);
}

static void runDay(void) {
    PlayerState *p=&g.players[g.currentPlayer];
    g.eventId=irand(0,6);
    float eventDemand=1.0f; int usable=p->stock; int effectiveCost=g.unitCostPence; int rep=0;
    
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
    p->unsold=p->stock-p->sold;
    p->revenuePence=p->sold*p->pricePence;
    p->costPence=p->stock*effectiveCost+p->adverts*150;
    p->profitPence=p->revenuePence-p->costPence;
    p->cashPence+=p->profitPence;

    if(p->sold==0)rep-=4;
    else{
        if(ratio<=1.05f)rep+=1;
        if(ratio>1.35f)rep-=3;
        if(p->sold>=usable&&p->customers>usable)rep-=1;
        if(p->unsold>p->stock*.45f)rep-=1;
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
        if(g.lang==LANG_EN) swprintf(buf,128,L"%s - DAY %d RESULTS",pName,g.day);
        else swprintf(buf,128,L"%s - %d일차 결과",pName,g.day);
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
        const wchar_t *titleStr;
        if(p0->cashPence<0) titleStr = g.lang==LANG_EN ? L"STALL CLOSED" : L"영업 종료 (파산)";
        else if(p0->cashPence+p0->reputation*45+p0->totalSold*5>=12000) titleStr = g.lang==LANG_EN ? L"CANDYFLOSS CHAMPION!" : L"솜사탕 챔피언!";
        else titleStr = g.lang==LANG_EN ? L"SEASON COMPLETE" : L"시즌 완수!";
        
        centreTextW(dc,(RECT){150,70,810,130},titleStr,RGB(37,58,99),fontLarge);
        
        wchar_t buf[128];
        moneyW(buf,128,p0->cashPence);
        centreTextW(dc,(RECT){210,180,460,245},buf,RGB(25,139,98),fontLarge);
        centreTextW(dc,(RECT){210,155,460,185},g.lang==LANG_EN?L"FINAL CASH":L"최종 자산",RGB(101,110,127),fontTiny);

        swprintf(buf,128,L"%d",p0->reputation);
        centreTextW(dc,(RECT){500,180,750,245},buf,RGB(40,161,229),fontLarge);
        centreTextW(dc,(RECT){500,155,750,185},g.lang==LANG_EN?L"REPUTATION":L"평판",RGB(101,110,127),fontTiny);

        swprintf(buf,128,L"%d",p0->totalSold);
        centreTextW(dc,(RECT){210,315,460,380},buf,RGB(235,71,139),fontLarge);
        centreTextW(dc,(RECT){210,290,460,320},g.lang==LANG_EN?L"TOTAL SOLD":L"총 판매량",RGB(101,110,127),fontTiny);

        moneyW(buf,128,p0->bestProfit);
        centreTextW(dc,(RECT){500,315,750,380},buf,RGB(235,154,42),fontLarge);
        centreTextW(dc,(RECT){500,290,750,320},g.lang==LANG_EN?L"BEST PROFIT":L"최대 일일이익",RGB(101,110,127),fontTiny);
    } else {
        centreTextW(dc,(RECT){150,60,810,115},g.lang==LANG_EN?L"FINAL LEADERBOARD":L"최종 순위표",RGB(37,58,99),fontLarge);

        int order[4]={0,1,2,3};
        int scores[4];
        for(int i=0;i<g.numPlayers;i++){
            scores[i]=g.players[i].cashPence + g.players[i].reputation*45 + g.players[i].totalSold*5;
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
            RECT rowR={140,130+i*80,820,195+i*80};
            fillRound(dc,rowR,18,i==0?RGB(255,248,220):RGB(242,247,252));

            RECT rankR={155,130+i*80,260,195+i*80};
            centreTextW(dc,rankR,g.lang==LANG_EN?ranksEN[i]:ranksKO[i],rankCols[i],fontSmall);

            RECT nameR={270,130+i*80,410,195+i*80};
            const wchar_t *pName = g.lang==LANG_EN ? playerNamesEN[pIdx] : playerNamesKO[pIdx];
            centreTextW(dc,nameR,pName,stallColours[pl->stallColour],fontMedium);

            wchar_t buf[64];
            moneyW(buf,64,pl->cashPence);
            RECT cashR={420,130+i*80,550,195+i*80};
            centreTextW(dc,cashR,buf,RGB(25,139,98),fontSmall);

            if(g.lang==LANG_EN) swprintf(buf,64,L"Rep: %d",pl->reputation);
            else swprintf(buf,64,L"평판: %d",pl->reputation);
            RECT repR={560,130+i*80,670,195+i*80};
            centreTextW(dc,repR,buf,RGB(40,150,220),fontSmall);

            if(g.lang==LANG_EN) swprintf(buf,64,L"Sold: %d",pl->totalSold);
            else swprintf(buf,64,L"판매: %d",pl->totalSold);
            RECT soldR={680,130+i*80,800,195+i*80};
            centreTextW(dc,soldR,buf,RGB(235,71,139),fontSmall);
        }
    }

    finalButtons[0].text = g.lang==LANG_EN ? L"PLAY AGAIN" : L"다시 시작";
    drawButtonW(dc,finalButtons[0],RGB(244,58,139));
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
        case SCREEN_PLAN:drawPlan(backDC);break;
        case SCREEN_RESULTS:drawResults(backDC);break;
        case SCREEN_FINAL:drawFinal(backDC);break;
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
                L"Check the forecast, choose stock, set a price and buy advertising. Finish the season with the most cash and reputation. Hotseat multiplayer allows 1 to 4 players to take turns!" :
                L"일기예보를 확인하고, 재고를 준비하고, 가격을 설정하고 홍보하세요. 시즌 종료 시 가장 많은 자산과 평판을 얻는 플레이어가 승리합니다!";
            const wchar_t *head = g.lang==LANG_EN ? L"How to play" : L"게임 설명";
            MessageBoxW(NULL,msg,head,MB_OK|MB_ICONINFORMATION);
        }
        else if(ptIn(titleButtons[2].r,x,y)){ g.soundOn=!g.soundOn; playSound(SOUND_CLICK); }
        else if(ptIn(titleButtons[3].r,x,y)){ g.lang=(g.lang==LANG_EN?LANG_KO:LANG_EN); playSound(SOUND_CLICK); }
    }
    else if(g.screen==SCREEN_SETUP){
        if(ptIn(setupButtons[1].r,x,y)){ playSound(SOUND_CLICK); g.screen=SCREEN_TITLE; return; }
        if(ptIn(setupButtons[0].r,x,y)){
            playSound(SOUND_COIN);
            g.day=1;
            for(int i=0;i<4;i++){
                g.players[i].active=(i<g.numPlayers);
                g.players[i].cashPence=g.difficulty==0?4500:g.difficulty==2?3000:3500;
                g.players[i].reputation=50;
                g.players[i].totalSold=0;
                g.players[i].bestProfit=-999999;
                if(g.numPlayers>1) g.players[i].stallColour=i;
            }
            prepareDay();
            if(g.numPlayers>1) g.screen=SCREEN_PASS_TURN;
            else g.screen=SCREEN_PLAN;
            return;
        }
        for(int i=0;i<4;i++){
            RECT r={180+i*140,140,305+i*140,185};
            if(ptIn(r,x,y)){ playSound(SOUND_CLICK); g.numPlayers=i+1; }
        }
        for(int i=0;i<3;i++){
            RECT r={205+i*190,240,355+i*190,285};
            if(ptIn(r,x,y)){ playSound(SOUND_CLICK); g.totalDays=i==0?7:i==1?10:14; }
            r=(RECT){205+i*190,340,355+i*190,385};
            if(ptIn(r,x,y)){ playSound(SOUND_CLICK); g.difficulty=i; }
        }
        if(g.numPlayers==1){
            for(int i=0;i<4;i++){
                RECT r={250+i*120,438,330+i*120,482};
                if(ptIn(r,x,y)){ playSound(SOUND_CLICK); g.players[0].stallColour=i; }
            }
        }
    }
    else if(g.screen==SCREEN_PASS_TURN){
        if(ptIn(passButtons[0].r,x,y)){
            playSound(SOUND_CLICK);
            g.screen=SCREEN_PLAN;
        }
    }
    else if(g.screen==SCREEN_PLAN){
        for(int i=0;i<3;i++)if(ptIn(sliders[i].r,x,y)){g.activeSlider=i;setSliderFromMouse(i,x);}
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
            else g.screen=SCREEN_PLAN;
        } else {
            if(g.day>=g.totalDays){
                playSound(SOUND_WIN);
                g.screen=SCREEN_FINAL;
            } else {
                g.day++;
                prepareDay();
                if(g.numPlayers>1) g.screen=SCREEN_PASS_TURN;
                else g.screen=SCREEN_PLAN;
            }
        }
    }
    else if(g.screen==SCREEN_FINAL&&ptIn(finalButtons[0].r,x,y)){
        playSound(SOUND_CLICK);
        g.screen=SCREEN_SETUP;
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
    switch(msg){
        case WM_CREATE:SetTimer(hwnd,TIMER_ID,33,NULL);return 0;
        case WM_SIZE:clientW=LOWORD(lp);clientH=HIWORD(lp);return 0;
        case WM_TIMER:g.tick++;InvalidateRect(hwnd,NULL,FALSE);return 0;
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
    g.screen=SCREEN_TITLE;g.lang=LANG_EN;g.numPlayers=1;g.totalDays=10;g.difficulty=1;g.soundOn=1;g.activeSlider=-1;
    g.players[0].stallColour=0;
    
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
