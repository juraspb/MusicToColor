unit FFTFilterConst;

interface

uses Windows, Math;

const
 BORDER  = 5;
 //FFTSIZE = {8192}{4096}{1024}512{256}{128};
 //FFTHALFSIZE = FFTSIZE div 2;
 //FFTPOW  = {13}{12}{10}9{8}{7};
 DELTA   = 3;
 //MAXPOINTCNT = 10*FFTSIZE;
 FD      = 11025;

type
 TGraphPoint = record
  iNum : Integer;
  Koord: TPoint;
  Amplitude: Real;
 end;
 PGraphPoint = ^TGraphPoint;
 TPointsArray = Array [0..0] of TGraphPoint;
 PPointsArray = ^TPointsArray;
 TPoints      = Array [0..0] of TPoint;
 PPoints =^TPoints;

 TPointArray = Array [0..0] of TPoint;
 PPointArray = ^TPointArray;

 //Sin cos table
 TTbl = Array [0..0] of Single;
 PTbl = ^TTbl;
 //-------------
 //array of complex
 TComplex =record
  Re, Im :Single;
  //Re, Im: double;
 end;
 PComplex = ^TComplex;
 TComplexArray = array [0..0] of TComplex;
 PComplexArray = ^TComplexArray;

 TSmallIntArray = Array [0..0] of SmallInt;
 PSmallIntArray = ^TSmallIntArray;

 TExtendedArray = Array [0..0] of Extended;
 PExtendedArray = ^TExtendedArray;

 TSingleArray = Array [0..0] of Single;
 PSingleArray = ^TSingleArray;

 TSmallIntArrayClass = class (TObject)
  private
   m_buff: PSmallIntArray;
   m_complex_item_size: Integer;
   FiItemCount: Integer;
   procedure SetItemCount(const Value: Integer);
   function GetItemCount(): Integer;
  public
   constructor Create(iLen: Integer);
   function getItem(ind: Integer): SmallInt;
   procedure setItem(ind: Integer; Value: SmallInt);
   destructor Destroy(); override;
   procedure fillZeroBuff;
   property ItemCount: Integer read GetItemCount write SetItemCount;
 end;
 PSmallIntArrayClass = ^TSmallIntArrayClass;

 TExtendedArrayClass = class (TObject)
  private
   m_buff: PExtendedArray;
   m_complex_item_size: Integer;
   FiItemCount: Integer;
   procedure SetItemCount(const Value: Integer);
   function GetItemCount(): Integer;
  public
   constructor Create(iLen: Integer);
   function getItem(ind: Integer): Extended;
   procedure setItem(ind: Integer; Value: Extended);
   procedure Normalize();
   destructor Destroy(); override;
   procedure fillZeroBuff;
   property ItemCount: Integer read GetItemCount write SetItemCount;
 end;
 PExtendedArrayClass = ^TExtendedArrayClass;

 TComplexArrayClass = class (TObject)
  private
   m_buff: PComplexArray;
   m_complex_item_size: Integer;
   FiItemCount: Integer;
   //FdoLogarifm: Boolean;
   procedure SetItemCount(const Value: Integer);
   function GetItemCount(): Integer;
    //procedure SetdoLogarifm(const Value: Boolean);
  public
   constructor Create(iLen: Integer);
   function getItem(ind: Integer): TComplex;
   procedure setItem(ind: Integer; Value: TComplex);
   procedure Normalize();
   procedure Square();
   procedure Logarifm();
   procedure getFirstProizv();

   procedure UpHiFreqHalf();
   procedure UpLoFreqHalf();
   procedure UpHiFreqFull();
   procedure UpLoFreqFull();

   procedure getProizv();
   procedure setToMel();

   procedure mul2Coef(coef: PExtendedArrayClass);

   destructor Destroy(); override;
   procedure fillZeroBuff;
   function copyToComplexArray(arr: PComplexArray; iOutTotalLen: Integer): Integer;
   function copyFromComplexArray(arr: PComplexArray; iTotalLen: Integer): Integer;
   property ItemCount: Integer read GetItemCount write SetItemCount;
   //property doLogarifm: Boolean read FdoLogarifm write SetdoLogarifm;
 end;
 PComplexArrayClass = ^TComplexArrayClass;

implementation

{ TComplexClassArray }

function TComplexArrayClass.copyFromComplexArray(arr: PComplexArray;
  iTotalLen: Integer): Integer;
var
 i: Integer;
 value: TComplex;
begin
 Result := 0;
 value.Re := 0;
 value.Im := 0;
 for i:=0 to GetItemCount()-1 do //reset out array data
  begin
   setItem(i, value);
  end;
 for i:=0 to iTotalLen-1 do //copy to out array data
  setItem(i, arr[i]);
end;

function TComplexArrayClass.copyToComplexArray(arr: PComplexArray;
  iOutTotalLen: Integer): Integer;
var
 i: Integer;
begin
 Result := 0;
 for i:=0 to iOutTotalLen-1 do //reset out array data
  begin
   arr[i].Re:=0;
   arr[i].Im:=0;
  end;
 for i:=0 to GetItemCount()-1 do //copy to out array data
  begin
   arr[i].Re := getItem(i).Re;
   arr[i].Im := getItem(i).Im;
  end;
end;

constructor TComplexArrayClass.Create(iLen: Integer);
begin
 SetItemCount(iLen);
 m_complex_item_size := GetItemCount()*sizeof(TComplex);
 GetMem(m_buff, m_complex_item_size);
 fillZeroBuff();
end;

destructor TComplexArrayClass.Destroy;
begin
 GetMem(m_buff, m_complex_item_size);
 inherited;
end;

procedure TComplexArrayClass.fillZeroBuff;
var
 i: Integer;
 value: TComplex;
begin
 value.Re := 0;
 value.Im := 0;
 for i:=0 to GetItemCount()-1 do //reset out array data
  begin
   setItem(i, value);
  end;
end;

procedure TComplexArrayClass.getFirstProizv;
var
 i: Integer;
 v: TComplex;
begin
 //Re, Im
 for i:=1 to GetItemCount()-1 do
  begin
   if getItem(i).Re<>0 then
    v.Re := (getItem(i).Re-getItem(i-1).Re)/2
   else
    v.Re := 0;

   if getItem(i).Im<>0 then
    v.Im := (getItem(i).Im-getItem(i-1).Im)/2
   else
    v.Im := 0;
   setItem(i, v);
  end;
end;

function TComplexArrayClass.getItem(ind: Integer): TComplex;
begin
 Result := m_buff[ind];
end;

function TComplexArrayClass.GetItemCount: Integer;
begin
 Result := FiItemCount;
end;

procedure TComplexArrayClass.getProizv;
var
 i: Integer;
 v: TComplex;
begin
 //Re, Im
 for i:=1 to GetItemCount() div 2-1 do
  begin
   v.Re := getItem(i).Re-getItem(i-1).Re;
   v.Im := getItem(i).Im-getItem(i-1).Im;
   setItem(i, v);
   setItem(GetItemCount()-1-i, v);
  end;
end;

procedure TComplexArrayClass.Logarifm;
var
 i: Integer;
 v: TComplex;
begin
 //Re, Im
 for i:=0 to GetItemCount()-1 do
  begin
   v.Re := Log10(1+getItem(i).Re);
   v.Im := Log10(1+getItem(i).Im);
   setItem(i, v);
  end;
end;

procedure TComplexArrayClass.mul2Coef(coef: PExtendedArrayClass);
var
 i: Integer;
 v: TComplex;
begin
 for i:=0 to coef.GetItemCount-1 do
  begin
   v.Re := getItem(i).Re*coef.getItem(i);
   v.Im := getItem(i).Im*coef.getItem(i);
   setItem(i, v);
   setItem(GetItemCount-1-i, v);
  end;
end;

procedure TComplexArrayClass.Normalize;
var
 mx: Extended;
 i: Integer;
 v: TComplex;
begin
 //Re
 mx := ABS(m_buff[0].Re);
 for i:=0 to GetItemCount()-1 do
  if mx<ABS(m_buff[i].Re) then
   mx := ABS(m_buff[i].Re);
 if mx=0 then
  mx := 1;
 for i:=0 to GetItemCount()-1 do
  begin
   v.Re := getItem(i).Re/mx;
   v.Im := getItem(i).Im;
   setItem(i, v);
  end;
 //Im
 mx := ABS(m_buff[0].Im);
 for i:=0 to GetItemCount()-1 do
  if mx<ABS(m_buff[i].Im) then
   mx := ABS(m_buff[i].Im);
 if mx=0 then
  mx := 1;
 for i:=0 to GetItemCount()-1 do
  begin
   v.Re := getItem(i).Re;
   v.Im := getItem(i).Im/mx;
   setItem(i, v);
  end;
end;

procedure TComplexArrayClass.setItem(ind: Integer; Value: TComplex);
begin
 m_buff[ind] := Value;
end;

procedure TComplexArrayClass.SetItemCount(const Value: Integer);
begin
 FiItemCount := Value;
end;

{ TExtendedArrayClass }

constructor TExtendedArrayClass.Create(iLen: Integer);
begin
 SetItemCount(iLen);
 m_complex_item_size := GetItemCount()*sizeof(Extended);
 GetMem(m_buff, m_complex_item_size);
 fillZeroBuff();
end;

destructor TExtendedArrayClass.Destroy;
begin
 GetMem(m_buff, m_complex_item_size);
 inherited;
end;

procedure TExtendedArrayClass.fillZeroBuff;
var
 i: Integer;
begin
 for i:=0 to GetItemCount()-1 do
  m_buff[i]:=0;
end;

function TExtendedArrayClass.getItem(ind: Integer): Extended;
begin
 Result := m_buff[ind];
end;

function TExtendedArrayClass.GetItemCount: Integer;
begin
 Result := FiItemCount;
end;

procedure TExtendedArrayClass.Normalize;
var
 mx: Extended;
 i: Integer;
 v: Extended;
begin
 v := 1;
 mx := ABS(m_buff[0]);
 for i:=0 to GetItemCount()-1 do
  if mx<ABS(m_buff[i]) then
   mx := ABS(m_buff[i]);
 if mx=0 then
  mx := 1;
 for i:=0 to GetItemCount()-1 do
  begin
   v := v/mx;
   setItem(i, v);
  end;
end;

procedure TExtendedArrayClass.setItem(ind: Integer; Value: extended);
begin
 m_buff[ind] := Value;
end;

procedure TExtendedArrayClass.SetItemCount(const Value: Integer);
begin
 FiItemCount := Value;
end;

{ TSmallIntArrayClass }

constructor TSmallIntArrayClass.Create(iLen: Integer);
begin
 SetItemCount(iLen);
 m_complex_item_size := GetItemCount()*sizeof(SmallInt);
 GetMem(m_buff, m_complex_item_size);
 fillZeroBuff();
end;

destructor TSmallIntArrayClass.Destroy;
begin
 GetMem(m_buff, m_complex_item_size);
 inherited;
end;

procedure TSmallIntArrayClass.fillZeroBuff;
var
 i: Integer;
begin
 for i:=0 to GetItemCount()-1 do
  m_buff[i]:=0;
end;

function TSmallIntArrayClass.getItem(ind: Integer): SmallInt;
begin
 Result := m_buff[ind];
end;

function TSmallIntArrayClass.GetItemCount: Integer;
begin
 Result := FiItemCount;
end;

procedure TSmallIntArrayClass.setItem(ind: Integer; Value: SmallInt);
begin
 m_buff[ind] := Value;
end;

procedure TSmallIntArrayClass.SetItemCount(const Value: Integer);
begin
 FiItemCount := Value;
end;

procedure TComplexArrayClass.setToMel;
var
 i: Integer;
 v: TComplex;
begin
 for i:=0 to GetItemCount()-1 do
  begin
   v.Re := 1595*Log10(1+getItem(i).Re/700);
   v.Im := 1595*Log10(1+getItem(i).Im/700);
   setItem(i, v);
  end;
end;

procedure TComplexArrayClass.Square;
var
 i: Integer;
 v: TComplex;
begin
 for i:=0 to GetItemCount()-1 do
  begin
   v.Re := getItem(i).Re;
   v.Im := getItem(i).Im;
   v.Re := sqrt(sqr(v.Re*v.Re+v.Im*v.Im));
   v.Im := 0;
   setItem(i, v);
  end;
end;

procedure TComplexArrayClass.UpHiFreqFull;
var
 i: Integer;
 v: TComplex;
 v_prv: TComplex;
begin
 v_prv.Re := 0;
 v_prv.Im := 0;
 for i:=0 to GetItemCount()-1 do
  begin
   v.Re := getItem(i).Re-0.9375*v_prv.Re;
   v_prv := getItem(i);
   setItem(i, v);
  end;
end;

procedure TComplexArrayClass.UpHiFreqHalf;
var
 i: Integer;
 v: TComplex;
 v_prv: TComplex;
begin
 v_prv.Re := 0;
 v_prv.Im := 0;
 for i:=0 to GetItemCount() div 2-1 do
  begin
   v.Re := getItem(i).Re-0.9375*v_prv.Re;
   v_prv := getItem(i);
   setItem(i, v);
   setItem(GetItemCount()-1-i, v);
  end;
end;

procedure TComplexArrayClass.UpLoFreqFull;
const
 a = 0.9375;
 //a = 0.1;
var
 i: Integer;
 v: TComplex;
 v_prv: TComplex;

 //w0, j: Integer;
 //s: Extended;
begin
 //w0 := GetItemCount();
 v_prv.Re := getItem(0).Re;
 v_prv.Im := 0;
 for i:=1 to GetItemCount()-1 do
  begin
   //v.Re := v_prv.Re-(0.9375)*(v_prv.Re-getItem(i).Re); //работает
   //v.Re := a*v_prv.Re-(1-a)*getItem(i).Re; //работает
   //v.Re := getItem(i).Re-0.9375*v_prv.Re;
   //+Простейший метод экспоненциального сглаживания
   v.Re := a*v_prv.Re-(1-a)*getItem(i).Re; //работает
   //-Простейший метод экспоненциального сглаживания
   v_prv := getItem(i);
   setItem(i, v);
  end;
end;

procedure TComplexArrayClass.UpLoFreqHalf;
begin
end;

end.
