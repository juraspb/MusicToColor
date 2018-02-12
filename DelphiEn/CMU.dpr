program CMU;

uses
  Forms,
  CMUru in 'CMUru.pas' {Form1},
  FFTBase in '..\unit\FFTBase.pas',
  FFTFilterConst in '..\unit\FFTFilterConst.pas',
  SerialLink in '..\unit\SerialLink.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
