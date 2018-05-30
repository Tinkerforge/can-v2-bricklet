program ExampleLoopback;

{ FIXME: This example is incomplete }

{$ifdef MSWINDOWS}{$apptype CONSOLE}{$endif}
{$ifdef FPC}{$mode OBJFPC}{$H+}{$endif}

uses
  SysUtils, IPConnection, BrickletCANV2;

type
  TExample = class
  private
    ipcon: TIPConnection;
    can: TBrickletCANV2;
  public
    procedure FrameReadCB(sender: TBrickletCANV2; const frameType: byte;
                          const identifier: longword; const data: TArrayOfUInt8);
    procedure Execute;
  end;

const
  HOST = 'localhost';
  PORT = 4223;
  UID = 'XYZ'; { Change XYZ to the UID of your CAN Bricklet 2.0 }

var
  e: TExample;

{ Callback procedure for frame read callback }
procedure TExample.FrameReadCB(sender: TBrickletCANV2; const frameType: byte;
                               const identifier: longword; const data: TArrayOfUInt8);
begin
  WriteLn(Format('Frame Type: %d', [frameType]));
  WriteLn(Format('Identifier: %d', [identifier]));
  WriteLn('');
end;

procedure TExample.Execute;
begin
  { Create IP connection }
  ipcon := TIPConnection.Create;

  { Create device object }
  can := TBrickletCANV2.Create(UID, ipcon);

  { Connect to brickd }
  ipcon.Connect(HOST, PORT);
  { Don't use device before ipcon is connected }

  { Configure transceiver for loopback mode }
  can.SetTransceiverConfiguration(1000000, 625,
                                  BRICKLET_CAN_V2_TRANSCEIVER_MODE_LOOPBACK);

  { Register frame read callback to procedure FrameReadCB }
  can.OnFrameRead := {$ifdef FPC}@{$endif}FrameReadCB;

  { Enable frame read callback }
  can.SetFrameReadCallbackConfiguration(true);

  WriteLn('Press key to exit');
  ReadLn;
  can.SetFrameReadCallbackConfiguration(false);
  ipcon.Destroy; { Calls ipcon.Disconnect internally }
end;

begin
  e := TExample.Create;
  e.Execute;
  e.Destroy;
end.