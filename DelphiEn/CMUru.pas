unit CMUru;

(*
  Copyright 2018, Petrukhanov Yuriy
  Email:juraspb@mail.ru
  Home: https://github.com/juraspb/MusictoColor
*)

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, fftbase, fftfilterconst, MMSystem, StdCtrls, ExtCtrls, Buttons,
  SerialLink, ColorGrd, ComCtrls, Spin, Registry, ShellApi, Menus,
  IdBaseComponent, IdComponent, IdTCPConnection, IdTCPClient, IdHTTP, jpeg;

Const
    BufSize = 1024;
    chan=2;
    fftSq = 10; // 2**9=512
    ffLen = 3;
    nBandPass = 20;
    CommunicPortSpeed = 115200;
    REGSTR_KEY_BCCNFG = '\SOFTWARE\JURASPB\CMU\Param';

type
  TModeDescr=record
    Channels: integer;
    Rate: integer;
    Bits: integer;
    mode: DWORD;          // код режима работы
    descr: string[32];    // словесное описание
end;

const
   colorTab: array [0..95] of TColor=(
      $FF0000,$FF1100,$FF2200,$FF3300,$FF4400,$FF5500,$FF6600,$FF7700,$FF8800,$FF9900,$FFAA00,$FFBB00,$FFCC00,$FFDD00,$FFEE00,$FFFF00,  //красный - жЄлтый
      $FFFF00,$EEFF00,$DDFF00,$CCFF00,$BBFF00,$AAFF00,$99FF00,$88FF00,$77FF00,$66FF00,$55FF00,$44FF00,$33FF00,$22FF00,$11FF00,$00FF00,  //жЄлтый Ч зелЄный
      $00FF00,$00FF11,$00FF22,$00FF33,$00FF44,$00FF55,$00FF66,$00FF77,$00FF88,$00FF99,$00FFAA,$00FFBB,$00FFCC,$00FFDD,$00FFEE,$00FFFF,  //зелЄный Ч циан (голубой)
      $00FFFF,$00EEFF,$00DDFF,$00CCFF,$00BBFF,$00AAFF,$0099FF,$0088FF,$0077FF,$0066FF,$0055FF,$0044FF,$0033FF,$0022FF,$0011FF,$0000FF,  //голубой Ч синий
      $0000FF,$1100FF,$2200FF,$3300FF,$4400FF,$5500FF,$6600FF,$7700FF,$8800FF,$9900FF,$AA00FF,$BB00FF,$CC00FF,$DD00FF,$EE00FF,$FF00FF,  //синий Ч пурпур (маджента)
      $FF00FF,$FF00EE,$FF00DD,$FF00CC,$FF00BB,$FF00AA,$FF0099,$FF0088,$FF0077,$FF0066,$FF0055,$FF0044,$FF0033,$FF0022,$FF0011,$FF0000); //маджента Ч красный

   Garmoniks : array [0..19] of Integer =(2,3,4,5,7,9,11,13,15,17,19,22,25,28,32,36,44,54,66,80);

   modes: array [1..12] of TModeDescr=((Channels: 1; Rate: 11025; Bits: 8; mode: WAVE_FORMAT_1M08; descr:'11.025 kHz, mono, 8-bit'),
                                       (Channels: 1; Rate: 11025; Bits: 16; mode: WAVE_FORMAT_1M16; descr:'11.025 kHz, mono, 16-bit'),
                                       (Channels: 2; Rate: 11025; Bits: 8; mode: WAVE_FORMAT_1S08; descr:'11.025 kHz, stereo, 8-bit'),
                                       (Channels: 2; Rate: 11025; Bits: 16; mode: WAVE_FORMAT_1S16; descr:'11.025 kHz, stereo, 16-bit'),
                                       (Channels: 1; Rate: 22050; Bits: 8; mode: WAVE_FORMAT_2M08; descr:'22.05 kHz, mono, 8-bit'),
                                       (Channels: 1; Rate: 22050; Bits: 16; mode: WAVE_FORMAT_2M16; descr:'22.05 kHz, mono, 16-bit'),
                                       (Channels: 2; Rate: 22050; Bits: 8; mode: WAVE_FORMAT_2S08; descr:'22.05 kHz, stereo, 8-bit'),
                                       (Channels: 2; Rate: 22050; Bits: 16; mode: WAVE_FORMAT_2S16; descr:'22.05 kHz, stereo, 16-bit'),
                                       (Channels: 1; Rate: 44100; Bits: 8; mode: WAVE_FORMAT_4M08; descr:'44.1 kHz, mono, 8-bit'),
                                       (Channels: 1; Rate: 44100; Bits: 16; mode: WAVE_FORMAT_4M16; descr:'44.1 kHz, mono, 16-bit'),
                                       (Channels: 2; Rate: 44100; Bits: 8; mode: WAVE_FORMAT_4S08; descr:'44.1 kHz, stereo, 8-bit'),
                                       (Channels: 2; Rate: 44100; Bits: 16; mode: WAVE_FORMAT_4S16; descr:'44.1 kHz, stereo, 16-bit'));

type
    TData16 = array [0..BufSize * chan - 1] of smallint;
    PData16 = ^TData16;
    TPointArr = array [0..BufSize * chan - 1] of TPoint;
    PPointArr = ^TPointArr;
    PDWORDArray = array of DWORD;

TForm1 = class(TForm)
    pgc1: TPageControl;
    ts1: TTabSheet;
    ts2: TTabSheet;
    pnl4: TPanel;
    img2: TImage;
    pnl5: TPanel;
    img1: TImage;
    pnl1: TPanel;
    lbl2: TLabel;
    trckbr1: TTrackBar;
    pm1: TPopupMenu;
    N1: TMenuItem;
    N2: TMenuItem;
    N3: TMenuItem;
    N4: TMenuItem;
    N31: TMenuItem;
    N5: TMenuItem;
    N6: TMenuItem;
    ts3: TTabSheet;
    lbl5: TLabel;
    lbl6: TLabel;
    btn19: TButton;
    lbl8: TLabel;
    lbl9: TLabel;
    btn12: TSpeedButton;
    ts4: TTabSheet;
    lbl3: TLabel;
    cbb1: TComboBox;
    chk2: TCheckBox;
    chk1: TCheckBox;
    ComboBox1: TComboBox;
    lbl4: TLabel;
    lbl1: TLabel;
    lst1: TListBox;
    lbl10: TLabel;
    se2: TSpinEdit;
    se1: TSpinEdit;
    btnD1: TSpeedButton;
    btnD2: TSpeedButton;
    btnD3: TSpeedButton;
    lbl13: TLabel;
    lbl12: TLabel;
    ts5: TTabSheet;
    img3: TImage;
    btn11: TSpeedButton;
    btn1: TSpeedButton;
    btn10: TSpeedButton;
    btn13: TSpeedButton;
    btn14: TSpeedButton;
    btn15: TSpeedButton;
    btn2: TSpeedButton;
    lbl14: TLabel;
    lbl15: TLabel;
    lbl16: TLabel;
    lbl17: TLabel;
    lbl19: TLabel;
    N7: TMenuItem;
    trckbr2: TTrackBar;
    lbl18: TLabel;
    btnD4: TSpeedButton;
    chk3: TCheckBox;
    lbl7: TLabel;
    btnD5: TSpeedButton;
    btnD6: TSpeedButton;
    SpeedButton3: TSpeedButton;
    SpeedButton4: TSpeedButton;
    btn9: TSpeedButton;
    SpeedButton5: TSpeedButton;
    SpeedButton6: TSpeedButton;
    SpeedButton7: TSpeedButton;
    SpeedButton8: TSpeedButton;
    SpeedButton9: TSpeedButton;
    btnD7: TSpeedButton;
    btnD8: TSpeedButton;
    btnD9: TSpeedButton;
    btnD10: TSpeedButton;
    btn3: TSpeedButton;
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormCreate(Sender: TObject);
    procedure ComboBox1Change(Sender: TObject);
    procedure cbb1Change(Sender: TObject);
    procedure btnD1Click(Sender: TObject);
    procedure trckbr1Change(Sender: TObject);
    procedure img2MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure N1Click(Sender: TObject);
    procedure N5Click(Sender: TObject);
    procedure btn12Click(Sender: TObject);
    procedure btn19Click(Sender: TObject);
    procedure lbl8Click(Sender: TObject);
    procedure chk2Click(Sender: TObject);
    procedure se1Change(Sender: TObject);
    procedure btn11Click(Sender: TObject);
    procedure chk3Click(Sender: TObject);
   protected
     procedure ControlWindow(Var Msg:TMessage); message WM_SYSCOMMAND;
     procedure IconMouse(var Msg:TMessage); message WM_USER+1;
   private
    { Private declarations }
     applicationsDir: string;
     FLink: TSerialLink;
     prog:byte;
     param: array[0..252] of byte;
     progInit: array[0..9] of byte;
     pozCount:Byte;
     initCnfg:boolean;
     mx,my,mcl:Integer;
     zcount,zbrigth:Integer;
     gain:Integer;
     eqBmp,wcBmp:TBitmap;
     CommunicPortName,audioDevicesName: string;
     procedure waveInit;
     procedure OnMinimizeProc(Sender:TObject);
     procedure ShowInfo;
  public
     audioDevices:Cardinal;
     inDataBuf: array [0..BufSize*chan-1] of SmallInt;
     fftDataBuf: array [0..BufSize*chan-1] of SmallInt;
     max10: array [0..9] of Integer;
     loc10: array [0..9] of Integer;
     bpFilter: array [0..ffLen-1,0..nBandPass-1] of Integer;
     clMagnBuf: array [0..nBandPass-1] of Integer;
     zcl: array [0..nBandPass-1] of Integer;
     zmuBuf: array [0..nBandPass-1] of byte;
     ffCount: Integer;
     SendBuff: array[0..31] of byte;
     RcvBuff: array[0..31] of Char;
     ledColor: array[0..9] of TColor;
     cmdString: string;
     sl:TStringList;
     r,g,b:Byte;
     state:Integer;
     alpha:Integer;
     brightness,rotate:byte;
     procedure Ic(n:Integer;Icon:TIcon);
     procedure OnWaveIn(var Msg: TMessage); message MM_WIM_DATA;
     procedure setBandPass;
     procedure programRun(number,val,br,rot:byte);
     procedure rcvAnswer;
     procedure MakeFFT;
     procedure DrawFFT_BP;
     procedure Connected;
     procedure stop;
     function LoadParams : boolean;
     function initParams : boolean;
     procedure storeParams;
     procedure SaveParams;
  end;

var
  Form1: TForm1;
  p:                     PPointArr;
  ready:                 boolean = false;
  mmfree:                boolean = false;
  Bits16:                boolean;
  hBuf:                  THandle;
  data16:                PData16;
  WaveIn:                hWaveIn;
  BufHead:               TWaveHdr;

implementation

{$R *.dfm}
procedure TForm1.OnMinimizeProc(Sender:TObject);
Begin
 PostMessage(Handle,WM_SYSCOMMAND,SC_MINIMIZE,0);
End;

procedure TForm1.ControlWindow(Var Msg:TMessage);
Begin
 IF Msg.WParam=SC_MINIMIZE then
  Begin
   Ic(1,Application.Icon);  //
   ShowWindow(Handle,SW_HIDE);  //
   ShowWindow(Application.Handle,SW_HIDE);  //
 End else inherited;
End;

procedure TForm1.Ic(n:Integer;Icon:TIcon);
Var Nim:TNotifyIconData;
begin
 With Nim do
  Begin
   cbSize:=SizeOf(Nim);
   Wnd:=Form1.Handle;
   uID:=1;
   uFlags:=NIF_ICON or NIF_MESSAGE or NIF_TIP;
   hicon:=Icon.Handle;
   uCallbackMessage:=wm_user+1;
   szTip:='÷ветомузыка';
  End;
 Case n OF
  1: Shell_NotifyIcon(Nim_Add,@Nim);
  2: Shell_NotifyIcon(Nim_Delete,@Nim);
  3: Shell_NotifyIcon(Nim_Modify,@Nim);
 End;
end;

procedure TForm1.IconMouse(var Msg:TMessage);
Var p:tpoint;
begin
 GetCursorPos(p); 
 Case Msg.LParam OF
//  WM_LBUTTONUP,WM_LBUTTONDBLCLK:
  WM_LBUTTONDBLCLK:
   Begin
    Ic(2,Application.Icon);
    ShowWindow(Application.Handle,SW_SHOW);
    ShowWindow(Handle,SW_SHOW);
   End;
  WM_RBUTTONUP:
   Begin
    SetForegroundWindow(Handle);
    pm1.Popup(p.X,p.Y);
    PostMessage(Handle,WM_NULL,0,0);
   end;
 End;
end;

procedure TForm1.DrawFFT_BP;
var i,j,k,zmax,nGarmoniks: integer;
    cl:TColor;
    r,g,b:Byte;
begin
  Inc(ffCount);
  if ffCount>ffLen-1 then ffCount:=0;
  // не равномерно по октавам
  i:=1;
  zmax:=0;
  for j:=0 to nBandPass-1 do
   begin
    nGarmoniks:=Garmoniks[j];
    clMagnBuf[j]:=0;
    for k:=0 to nGarmoniks-1 do
     begin
       clMagnBuf[j]:=clMagnBuf[j]+fftDataBuf[i]+fftDataBuf[BufSize-i];
       inc(i);
     end;
    bpFilter[ffCount,j]:=clMagnBuf[j]div 3;
    zcl[j]:=0;
    for k:=0 to ffLen-1 do zcl[j]:=zcl[j]+bpFilter[k,j];
    zcl[j]:=zcl[j] div ffLen;
    if zcl[j]>255 then zcl[j]:=255;
    if zcl[j]>zmax then zmax:=zcl[j];
   end;
  eqBmp.Canvas.Brush.Color:=clBlack;
  eqBmp.Canvas.FillRect(eqBmp.Canvas.ClipRect);
  for j:=0 to nBandPass-1 do
    begin
      cl:=colorTab[96 * j div nBandPass];
      b:=cl;
      g:=cl shr 8;
      r:=cl shr 16;
      eqBmp.Canvas.Brush.Color:=RGB(r,g,b);
      eqBmp.Canvas.FillRect(Rect(j*8,255-zcl[j],j*8+7,255));
    end;
  img1.Picture.Bitmap.Assign(eqBmp);
  if (zmax<8)and(prog<236) then
   begin
     if zcount=0 then
      begin
        if zbrigth<256 then Inc(zbrigth);
        for j:=0 to nBandPass-1 do zmuBuf[j]:=zbrigth div 4;
      end else
      begin
        zbrigth:=0;
        dec(zcount);
      end;
   end else
   begin
     zcount:=10;
     for j:=0 to nBandPass-1 do zmuBuf[j]:=zcl[j];
   end;
  if FLink.Active then setBandPass;
end;

procedure TForm1.ShowInfo;
var
   WaveNums, i, j: integer;
   WaveInCaps: TWaveInCaps;
begin
  j:=0;
  WaveNums:=waveInGetNumDevs;
  if WaveNums>0 then
   begin
    for i:=0 to WaveNums-1 do
     begin
      waveInGetDevCaps(i,@WaveInCaps,sizeof(TWaveInCaps));
      cbb1.Items.Add(PChar(@WaveInCaps.szPname));
      if audioDevicesName=cbb1.Items[i] then j:=i;
     end;
    cbb1.ItemIndex:=j;
    cbb1.OnChange(self);
   end;
end;

procedure TForm1.MakeFFT;
var
  fftb: TFFTBase; //FFT
  fFFTComplBuf: ^TComplexArray;
  i: integer;
begin
  GetMem(fFFTComplBuf, BufSize*SizeOf(TComplex));
  for i:=0 to BufSize-1 do
    begin
      fFFTComplBuf[i].Re := (inDataBuf[i*chan]+inDataBuf[i*chan+1])div 2;
      fFFTComplBuf[i].Im := 0;
    end;
  fftb:=TFFTBase.Create(nil);
  //FFT
  //Set:
  //N
  //2^X=N
  //False Ц FFT; True Ц iFFT
  //Window: 0, 1, 2, 3, 4
  fftb.FFT(Pointer(fFFTComplBuf), BufSize, fftSq, False, 0);
  for i:=0 to BufSize-1 do fftDataBuf[i] := Round(fFFTComplBuf[i].Re / gain);
  fftb.Free;
  FreeMem(fFFTComplBuf, BufSize*SizeOf(TComplex)); //ќсвобождение пам€ти выделенной под массив
end;

procedure TForm1.OnWaveIn;
var i: integer;
begin
  Data16 := PData16(PWaveHdr(Msg.lParam)^.lpData);
  for i := 0 to BufSize*chan-1 do inDataBuf[i]:= Data16^[i];
  MakeFFT();
  DrawFFT_BP();
  if ready then WaveInAddBuffer(WaveIn, PWaveHdr(Msg.lParam), SizeOf(TWaveHdr))
           else mmfree := true;
end;

procedure TForm1.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  if ready then stop;
  storeParams;
  SaveParams;
  sl.Free;
  FLink.Free;
  eqBmp.Free;
  wcBmp.Free;
end;

procedure TForm1.stop;
begin
   ready := false;
   while not mmfree do Application.ProcessMessages;
   mmfree := false;
   WaveInReset(WaveIn);
   WaveInUnPrepareHeader(WaveIn, addr(BufHead), sizeof(BufHead));
   WaveInClose(WaveIn);
   GlobalUnlock(hBuf);
   GlobalFree(hBuf);
   FreeMem(p, BufSize * sizeof(TPoint));
end;

procedure TForm1.storeParams;
var sl:TStringList;
    i:Integer;
begin
  sl:=TStringList.Create;
  for i:=0 to 252 do sl.Add(IntToStr(param[i]));
  sl.SaveToFile('param.ini');
  sl.Free;
end;


function TForm1.initParams : boolean;
var sl:TStringList;
    fn:TFileName;
    i:Integer;
begin
  fn:='param.ini';
  if FileExists(fn) then
   begin
    sl:=TStringList.Create;
    sl.LoadFromFile(fn);
    for i:=0 to 252 do param[i] := strtoint(sl.Strings[i]);
    sl.Free;
    result:=True;
   end else result:=false;
end;

function TForm1.LoadParams : boolean;
var
  Reg: TRegistry;
  ch:string;
begin
  Reg := TRegistry.Create;
  try
    Result:=false;
    if Reg.OpenKey(REGSTR_KEY_BCCNFG, False) then
    begin
     if Reg.ValueExists('cfgCOM') then CommunicPortName:= Reg.ReadString('cfgCOM')
                                  else CommunicPortName:= 'COM8';
     if Reg.ValueExists('cfgAudio') then audioDevicesName:= Reg.ReadString('cfgAudio')
                                    else audioDevicesName:= '';
     if Reg.ValueExists('cfgTree') then ch:= Reg.ReadString('cfgTree')
                                   else ch:= '0';
     if ch='1' then chk1.Checked:=True;
     if Reg.ValueExists('cfgProg') then prog:=Reg.ReadInteger('cfgProg')
                                   else prog:=0;
     if Reg.ValueExists('cfgGain') then gain:= Reg.ReadInteger('cfgGain')
                                   else gain:= 1024;
     if Reg.ValueExists('cfgBrightness') then Brightness:= Reg.ReadInteger('cfgBrightness')
                                         else Brightness:= 128;
     if Reg.ValueExists('cfgAutoRun') then ch:= Reg.ReadString('cfgAutoRun')
                                      else ch:= '0';
     if Reg.ValueExists('cfgProg1') then progInit[0]:= Reg.ReadInteger('cfgProg1')
                                    else progInit[0]:= 0;
     if Reg.ValueExists('cfgProg2') then progInit[1]:= Reg.ReadInteger('cfgProg2')
                                    else progInit[1]:= 1;
     if Reg.ValueExists('cfgProg3') then progInit[2]:= Reg.ReadInteger('cfgProg3')
                                    else progInit[2]:= 2;
     if Reg.ValueExists('cfgProg4') then progInit[3]:= Reg.ReadInteger('cfgProg4')
                                    else progInit[3]:= 3;
     if Reg.ValueExists('cfgProg5') then progInit[4]:= Reg.ReadInteger('cfgProg5')
                                    else progInit[4]:= 4;
     if Reg.ValueExists('cfgProg6') then progInit[5]:= Reg.ReadInteger('cfgProg6')
                                    else progInit[5]:= 5;
     if Reg.ValueExists('cfgProg7') then progInit[6]:= Reg.ReadInteger('cfgProg7')
                                    else progInit[6]:= 6;
     if Reg.ValueExists('cfgProg8') then progInit[7]:= Reg.ReadInteger('cfgProg8')
                                    else progInit[7]:= 7;
     if Reg.ValueExists('cfgProg9') then progInit[8]:= Reg.ReadInteger('cfgProg9')
                                    else progInit[8]:= 8;
     if Reg.ValueExists('cfgProg10') then progInit[9]:= Reg.ReadInteger('cfgProg10')
                                     else progInit[9]:= 9;
     if ch='1' then chk2.Checked:=True;
     Result:=true;
    end;
  finally
    Reg.Free;
  end;
end;

procedure TForm1.SaveParams;
var Reg: TRegistry;
begin
    Reg := TRegistry.Create;
    try
     if Reg.OpenKey(REGSTR_KEY_BCCNFG, True) then
      begin
        Reg.WriteString('cfgCOM',CommunicPortName);
        Reg.WriteString('cfgAudio',audioDevicesName);
        if chk1.Checked then Reg.WriteString('cfgTree','1')
                        else Reg.WriteString('cfgTree','0');
        Reg.WriteInteger('cfgProg',prog);
        Reg.WriteInteger('cfgGain',gain);
        Reg.WriteInteger('cfgBrightness',Brightness);
        if chk2.Checked then Reg.WriteString('cfgAutoRun','1')
                        else Reg.WriteString('cfgAutoRun','0');
        Reg.WriteInteger('cfgProg1',progInit[0]);
        Reg.WriteInteger('cfgProg2',progInit[1]);
        Reg.WriteInteger('cfgProg3',progInit[2]);
        Reg.WriteInteger('cfgProg4',progInit[3]);
        Reg.WriteInteger('cfgProg5',progInit[4]);
        Reg.WriteInteger('cfgProg6',progInit[5]);
        Reg.WriteInteger('cfgProg7',progInit[6]);
        Reg.WriteInteger('cfgProg8',progInit[7]);
        Reg.WriteInteger('cfgProg9',progInit[8]);
        Reg.WriteInteger('cfgProg10',progInit[9]);
      end;
    finally
      Reg.CloseKey();
      Reg.Free;
    end;
end;

procedure TForm1.FormCreate(Sender: TObject);
var i,x,y,j:Integer;
    cl:TColor;
begin
  GetDir(0,applicationsDir);
  chDir(applicationsDir);
  sl:=TStringList.Create;
  Application.onMinimize:=OnMinimizeProc;
  eqBmp:=TBitmap.Create;
  wcBmp:=TBitmap.Create;
  eqBmp.Width:=20*8;
  eqBmp.Height:=256;
  wcBmp.Width:=8;
  wcBmp.Height:=8;
  for i:=0 to 47 do
   begin
     cl:=colorTab[i*2];
     b:=cl;
     g:=cl shr 8;
     r:=cl shr 16;
     y:=i div 8;
     x:=i mod 8;
     if (y mod 2)=0 then wcBmp.Canvas.Pixels[y,x]:=RGB(r,g,b)
                    else wcBmp.Canvas.Pixels[y,7-x]:=RGB(r,g,b);
   end;
  for i:=48 to 63 do
   begin
     y:=i div 8;
     x:=i mod 8;
     j:=(64-i)* 16 - 1;
     if (y mod 2)=0 then wcBmp.Canvas.Pixels[y,x]:=RGB(j,j,j)
                    else wcBmp.Canvas.Pixels[y,7-x]:=RGB(j,j,j);
   end;
  img2.Picture.Bitmap.Assign(wcBmp);
  ready := false;
  prog := 2;
  zcount:=0;
  pozCount:=0;
  rotate:=0;
  if not LoadParams then
   begin
    CommunicPortName:='COM8';
    prog:=2;
    gain:=1024;
    Brightness:=128;
   end else
   begin
     btnD1.Caption:=inttostr(progInit[0]div 8)+inttostr(progInit[0] and 7);
     btnD2.Caption:=inttostr(progInit[1]div 8)+inttostr(progInit[1] and 7);
     btnD3.Caption:=inttostr(progInit[2]div 8)+inttostr(progInit[2] and 7);
     btnD4.Caption:=inttostr(progInit[3]div 8)+inttostr(progInit[3] and 7);
     btnD5.Caption:=inttostr(progInit[4]div 8)+inttostr(progInit[4] and 7);
     btnD6.Caption:=inttostr(progInit[5]div 8)+inttostr(progInit[5] and 7);
     btnD7.Caption:=inttostr(progInit[6]div 8)+inttostr(progInit[6] and 7);
     btnD8.Caption:=inttostr(progInit[7]div 8)+inttostr(progInit[7] and 7);
     btnD9.Caption:=inttostr(progInit[8]div 8)+inttostr(progInit[8] and 7);
     btnD10.Caption:=inttostr(progInit[9]div 8)+inttostr(progInit[9] and 7);
   end;
  if not initParams then
   begin
    for i:=0 to 252 do param[i] := 128;
   end;
  ComboBox1.Text:=CommunicPortName;
  Connected;
  trckbr1.Position:=Byte(param[prog]);
  trckbr2.Position:=Byte(brightness);
  ShowInfo;
  if FLink.Active then
   begin
    if prog<232 then
     begin
       initCnfg:=True;
       se2.Value:=prog div 8;
       se1.Value:=prog and 7;
       pgc1.TabIndex := 1;
       initCnfg:=False;
       sleep(1500);
       programRun(prog,param[prog],brightness,rotate);
     end else waveInit;
    if chk1.Checked then PostMessage(Handle,WM_SYSCOMMAND,SC_MINIMIZE,0);
   end;
end;

procedure TForm1.Connected;
begin
  try
    if FLink = nil then
     begin
      FLink:= TSerialLink.Create(nil);
     end;
    FLink.Port:= CommunicPortName;
    FLink.Speed:= CommunicPortSpeed;
    FLink.Open;
   finally
    if not FLink.Active then
     begin
      Lbl1.Font.Color:=clRed;
      Lbl1.Caption:='Closed';
      MessageDlg('No connection', mtError, [mbOK], 0);
     end else
     begin
      Lbl1.Font.Color:=clGreen;
      Lbl1.Caption:='Opened';
     end;
  end;
end;

procedure TForm1.ComboBox1Change(Sender: TObject);
begin
   WITH Sender AS TComboBox DO
    begin
     case tag of
       0 : begin
            CommunicPortName:=Text;
            FLink.Port:= CommunicPortName;
           end;
       end;
     if not FLink.Active then Connected;
    end;
end;

procedure TForm1.waveInit;
var BufLen: word;
    buf: pointer;
    header: TWaveFormatEx;
begin
  with header do
    begin
      wFormatTag := WAVE_FORMAT_PCM;
      nChannels := chan;
      nSamplesPerSec := 44100;
      wBitsPerSample := 16 ;
      nBlockAlign := nChannels * (wBitsPerSample div 8);
      nAvgBytesPerSec := nSamplesPerSec * nBlockAlign;
      cbSize := 0;
    end;
  audioDevices:=cbb1.ItemIndex;
//  WaveInOpen(Addr(WaveIn), WAVE_MAPPER, addr(header), Form1.Handle, 0, CALLBACK_WINDOW);  // ѕодключение к устройству ввода выбранному системой WAVE_MAPPER
  WaveInOpen(Addr(WaveIn), audioDevices, addr(header), Form1.Handle, 0, CALLBACK_WINDOW);
  BufLen := header.nBlockAlign * BufSize;
  hBuf := GlobalAlloc(GMEM_MOVEABLE and GMEM_SHARE, BufLen);
  Buf := GlobalLock(hBuf);
  with BufHead do
    begin
      lpData := Buf;
      dwBufferLength := BufLen;
      dwFlags := WHDR_BEGINLOOP;
    end;
  WaveInPrepareHeader(WaveIn, Addr(BufHead), sizeof(BufHead));
  WaveInAddBuffer(WaveIn, addr(BufHead), sizeof(BufHead));
  GetMem(p, BufSize * sizeof(TPoint));
  ready := true;
  WaveInStart(WaveIn);
end;

procedure TForm1.cbb1Change(Sender: TObject);
var
   i: integer;
   WaveInCaps: TWaveInCaps;
begin
   if ready then stop;
   lst1.Clear;
   audioDevicesName:=cbb1.Items[cbb1.ItemIndex];
   for i:=1 to High(modes) do
    begin
     waveInGetDevCaps(cbb1.ItemIndex,@WaveInCaps,sizeof(TWaveInCaps));
     if (modes[i].mode and WaveInCaps.dwFormats)=modes[i].mode then lst1.Items.Add(modes[i].descr);
    end;
end;


procedure TForm1.setBandPass;
var i:Integer;
begin
   SendBuff[0]:=253;           // Start command
   for i:=0 to nBandPass-1 do
    begin
     if zmuBuf[i]>252 then zmuBuf[i]:=255;
     SendBuff[i+1]:=zmuBuf[i]; // BandPass magnitude
    end;
   SendBuff[21]:=prog;         // Programm number
   SendBuff[22]:=param[prog];  // Gain
   SendBuff[23]:=brightness;   // Brightness
   SendBuff[24]:=0;            // Control
   SendBuff[25]:=254;          // End command
   FLink.SendBuffer(SendBuff);
   rcvAnswer;                  // Answer
end;

procedure TForm1.btnD1Click(Sender: TObject);
var State : TKeyboardState;
begin
   if ready then stop;
   with Sender as TSpeedButton do
    begin
      GetKeyboardState(State);
      if(State[vk_Shift] and 128)<>0 then
        begin
          prog:=se2.Value*8+se1.Value;
          progInit[tag]:=prog;
          Caption:=inttostr(se2.Value)+inttostr(se1.Value);
        end else
        begin
          prog:=progInit[tag];
        end;
       programRun(prog,param[prog],brightness,0);
       Sleep(200);
       trckbr1.Position:=param[prog];
    end;
end;

procedure TForm1.programRun(number,val,br,rot:byte);
var i:Integer;
begin
    SendBuff[0]:=253;      // Start command
    for i:=1 to 20 do SendBuff[i]:=number;
    SendBuff[21]:=number;  // Programm number
    SendBuff[22]:=val;     // speed or gain
    SendBuff[23]:=br;      // Brightness
    SendBuff[24]:=rot;     // Control
    SendBuff[25]:=254;     // End command
    FLink.SendBuffer(SendBuff);
    rcvAnswer;             // Answer
end;

procedure TForm1.rcvAnswer;
var receive_byte,i:integer;
    s_in:string;
begin
  if FLink.Active then
   begin
    receive_byte:=FLink.ReceiveInQue;
    if receive_byte>0 then
     begin
      FLink.ReceiveBuffer(RcvBuff,receive_byte);
      s_in:='';
      for i:=0 to receive_byte-1 do
       Begin
        s_in:=s_in+inttohex(Byte(RcvBuff[i]),2)+',';
//        s_in:=s_in+RcvBuff[i];
       end;
       sl.Add(s_in);
       if sl.Count>100 then
        begin
         sl.SaveToFile('answ.txt');
         sl.Clear;
        end;
     end;
   end;
end;

procedure TForm1.trckbr1Change(Sender: TObject);
begin
  with Sender as TTrackBar do
   begin
     if Tag=0 then
      begin
        param[prog]:=trckbr1.Position;
        lbl2.Caption:=IntToStr(param[prog]);
        if pgc1.TabIndex = 1 then
         begin
           programRun(255,param[prog],brightness,rotate); // 255 - change the gain, brightness, mode
           Sleep(200);
         end else gain:=64+(252-param[prog])*8;   // change the gain
      end else
      begin
        brightness:=trckbr2.Position;
        lbl18.Caption:=IntToStr(brightness);
        if pgc1.TabIndex = 1 then
         begin
           programRun(255,param[prog],brightness,rotate);
           Sleep(200);
         end;
      end;
   end;
end;

procedure TForm1.img2MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  mx:=8*X div img2.Width;
  my:=8*Y div img2.Height;
  if ready then stop;
  prog:=252;
  if (mx mod 2)=0 then mcl:=mx*8+my
                  else mcl:=mx*8+7-my;
  param[prog]:=mcl;
  programRun(prog,param[prog],brightness,rotate);   // Set strip led color
end;

procedure TForm1.N1Click(Sender: TObject);
begin
  with Sender as TMenuItem do
   begin
     if Tag<252 then
      begin
        prog:=tag;
        trckbr1.Position:=param[prog];
        if not ready then waveInit;
      end else
      begin
        prog:=tag;
        if Tag=252 then param[prog]:=63;
        programRun(prog,param[prog],brightness,rotate);
      end;
   end;
end;

procedure TForm1.N5Click(Sender: TObject);
begin
   SaveParams;
   Application.Terminate;
end;

procedure TForm1.btn12Click(Sender: TObject);
begin
  if ready then stop;
  Sleep(200);
  prog:=252;
  param[prog]:=63;
  programRun(prog,param[prog],brightness,rotate);
end;

procedure TForm1.btn19Click(Sender: TObject);
begin
  ShellExecute(Application.Handle,'open','https://1drv.ms/u/s!AnhvZp98C-GCo2SBgxaOUuz-ycCR',nil,nil,0);
end;

procedure TForm1.lbl8Click(Sender: TObject);
begin
   with Sender as TLabel do
    begin
      case Tag of
         0: ShellExecute(Application.Handle,'open','https://paypal.me/Petrukhanov',nil,nil,0);
         1: ShellExecute(Application.Handle,'open','https://qiwi.com/payment/form/99',nil,nil,0);
         2: ShellExecute(Application.Handle,'open','https://www.webmoney.ru/',nil,nil,0);
         3: ShellExecute(Application.Handle,'open','https://money.yandex.ru/to/41001598825682',nil,nil,0);
         4: ShellExecute(Application.Handle,'open','https://mail.ru','juraspb@mail.ru',nil,0);
       end;
     end;
end;

procedure TForm1.chk2Click(Sender: TObject);
var Reg:TRegistry;
begin
  if chk2.Checked then
  begin
     Reg := TRegistry.Create;
     Reg.RootKey := HKEY_CURRENT_USER;
     Reg.OpenKey('\SOFTWARE\Microsoft\Windows\CurrentVersion\Run', false);
     Reg.WriteString('JURASPB', Application.ExeName);
     Reg.Free;
  end else
  begin
     Reg := TRegistry.Create;
     Reg.RootKey := HKEY_CURRENT_USER;
     Reg.OpenKey('\SOFTWARE\Microsoft\Windows\CurrentVersion\Run',false);
     Reg.DeleteValue('JURASPB');
     Reg.Free;
  end;
end;

procedure TForm1.se1Change(Sender: TObject);
begin
   if initCnfg then Exit;
   if ready then stop;
   prog:=se2.Value*8+se1.Value;
   programRun(prog,param[prog],brightness,0);
   Sleep(200);
   trckbr1.Position:=param[prog];
end;

procedure TForm1.btn11Click(Sender: TObject);
begin
  zbrigth:=0;
  with Sender as TSpeedButton do
   begin
     prog:=tag;
     trckbr1.Position:=param[prog];
   end;
  if ready then Exit;
  waveInit;
end;

procedure TForm1.chk3Click(Sender: TObject);
begin
   if chk3.Checked then rotate:=255                // random choise on
                   else rotate:=0;                 // random choise off
   programRun(255,param[prog],brightness,rotate);  // send command
   Sleep(200);
end;

end.
