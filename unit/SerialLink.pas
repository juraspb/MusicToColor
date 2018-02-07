unit SerialLink;
(*
  Copyright 2018, Petrukhanov Yuriy
  Email:juraspb@mail.ru
  Home: https://github.com/juraspb/MusictoColor
*)

interface

uses
  Classes, Windows, SysUtils;

type
  TOnErrorEvent = procedure(Sender: TObject; const Msg: string) of object;

  TSerialLink = class(TComponent)
  private
    FHandle: Cardinal;
    FActive: Boolean;
    FPort: string;
    FSpeed: integer;
    FError: integer;
    FOnError: TOnErrorEvent;
    FOverLapped: OVERLAPPED;
    procedure SetActive(val: Boolean);
    procedure SetPort(val: string);
    procedure SetSpeed(val: integer);
  protected
    procedure DoErrorEvent(const Msg: string);
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    property Active: Boolean read FActive write SetActive;
    property Port: string read FPort write SetPort;
    property Speed: integer read FSpeed write SetSpeed;
    property Error: integer read FError;
    procedure Open;
    procedure Close;
    procedure SendBuffer(var SendBuff: array of byte);
    procedure ReceiveBuffer(var RcvBuff: array of char; ToReceive: Cardinal);
    function ReceiveInQue: integer;
    function SetTimeouts(ReadTotal: Cardinal): Boolean;
    property OnError: TOnErrorEvent read FOnError write FOnError;
  end;

implementation
{ TSerialLink }

const
  MAX_TRYNMB = 1;

procedure Delay(tiks : longint);
var  TCount,T1Count: longint;
begin
  TCount:=GetTickCount;
  Repeat
   T1Count:=GetTickCount;
   if TCount > T1Count then
   Begin
    TCount:=GetTickCount;
    T1Count:=GetTickCount;
   End;
  Until ((T1Count-TCount) > tiks);
end;

procedure TSerialLink.Close;
begin
  if not Active then Exit;
  try
    CloseHandle(FHandle);
  finally
    FActive:= False;
  end;
end;

constructor TSerialLink.Create(AOwner: TComponent);
begin
  inherited;
  FActive:= False;
  FError:= 0;
  FPort:= 'COM1';
  FSpeed:= 2400;
end;


destructor TSerialLink.Destroy;
begin
  Close;
  inherited;
end;

procedure TSerialLink.DoErrorEvent(const Msg: string);
begin
  if Assigned(OnError) then OnError(Self, Msg);
end;

procedure TSerialLink.Open;
var _DCB : TDCB;
begin
  {Откроем порт}
  FHandle := CreateFile(PChar(Port), GENERIC_READ+GENERIC_WRITE, 0, nil,
               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if FHandle = INVALID_HANDLE_VALUE then
  begin
    DoErrorEvent('Не могу открыть порт');
    FError:= -1;
    Exit;
  end;
 try
  {Установить параметры порта}
  if not GetCommState(FHandle, _DCB) then
  begin
    DoErrorEvent('Не могу получить состояние порта');
    FError:= -2;
    Exit;
  end;
  with _DCB do
  begin
    BaudRate := FSpeed;
    Flags := $00000001;
    ByteSize := DATABITS_8;
    Parity := NOPARITY;
    StopBits := TWOSTOPBITS;
  end;
  if not SetCommState(FHandle, _DCB) then
  begin
    DoErrorEvent('Не могу установить состояние порта');
    FError:= -3;
    Exit;
  end;
  EscapeCommFunction(FHandle, SETDTR);
  if not SetTimeouts(10000) then Exit;
  FActive:= True;
  FError:= 0;
 finally
   if not FActive then CloseHandle(FHandle);
 end;
end;

procedure TSerialLink.SetActive(val: Boolean);
begin
  if val then Open else Close;
end;

procedure TSerialLink.SetPort(val: string);
begin
  if FPort = val then Exit;
  FPort:= val;
  if FActive then
  begin
    Close;
    Open;
  end;
end;

procedure TSerialLink.SetSpeed(val: integer);
begin
  if FSpeed = val then Exit;

  FSpeed:= val;
  if FActive then
  begin
    Close;
    Open;
  end;
end;

function TSerialLink.SetTimeouts(ReadTotal: Cardinal): Boolean;
var
  _CommTimeouts : TCommTimeouts;
begin
  Result:= False;
  with _CommTimeouts do
  begin
//    ReadIntervalTimeout:= 20;
    ReadIntervalTimeout:= 0;
    ReadTotalTimeoutMultiplier:= 4;
    ReadTotalTimeoutConstant:= ReadTotal;
    WriteTotalTimeoutMultiplier:= 0;
    WriteTotalTimeoutConstant:= 0;
  end;
  if not SetCommTimeouts(FHandle, _CommTimeouts) then
  begin
    DoErrorEvent('Не могу установить таймаут порта');
    Exit;
  end;
  Result:= True;
end;

procedure TSerialLink.SendBuffer(var SendBuff: array of byte);
var
  written,ToSend,i: Cardinal;
  trynmb: Cardinal;
begin
  ToSend := 0;
  while (SendBuff[ToSend]<>254) do ToSend:=ToSend+1;
  ToSend:=ToSend+1;
  if ToSend>0 then
   begin
    for trynmb:= 1 to MAX_TRYNMB do
     begin
      if not WriteFile(FHandle, SendBuff, ToSend, written, nil) Or (written <> ToSend) then
      begin
        DoErrorEvent('Не могу записать в порт');
        FError:= -4;
        Exit;
      end;
     end;
    FlushFileBuffers(FHandle);
    FError:= 0;
   end;
end;

procedure TSerialLink.ReceiveBuffer(var RcvBuff: array of char; ToReceive: Cardinal);
var
  rcvd: Cardinal;
begin
//  PurgeComm(FHandle,PURGE_RXCLEAR);
  if not ReadFile(FHandle, RcvBuff, ToReceive, rcvd, nil) then
   begin
    DoErrorEvent('Не могу прочитать из порта');
    FError:= -5;
    Exit;
   end;
  if rcvd = 0 then
   begin
    DoErrorEvent('Пустой буфер порта');
    FError:= -6;
    Exit;
   end;
  PurgeComm(FHandle,PURGE_TXCLEAR+PURGE_RXCLEAR);
  FError:= 0;
end;

function TSerialLink.ReceiveInQue : integer;
var ComSt: TComStat;
    ComErrors: dword;
Begin
  ClearCommError(FHandle,ComErrors,Addr(ComSt));
  result:=ComSt.cbInQue;
End;

end.
