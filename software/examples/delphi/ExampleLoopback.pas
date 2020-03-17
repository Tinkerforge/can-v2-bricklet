program ExampleLoopback;

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
var i: integer;
begin
  if (frameType = BRICKLET_CAN_V2_FRAME_TYPE_STANDARD_DATA) then begin
    WriteLn('Frame Type: Standard Data');
  end
  else if (frameType = BRICKLET_CAN_V2_FRAME_TYPE_STANDARD_REMOTE) then begin
    WriteLn('Frame Type: Standard Remote');
  end
  else if (frameType = BRICKLET_CAN_V2_FRAME_TYPE_EXTENDED_DATA) then begin
    WriteLn('Frame Type: Extended Data');
  end
  else if (frameType = BRICKLET_CAN_V2_FRAME_TYPE_EXTENDED_REMOTE) then begin
    WriteLn('Frame Type: Extended Remote');
  end;

  WriteLn(Format('Identifier: %d', [identifier]));
  Write(Format('Data (Length: %d):', [Length(data)]));

  for i := 0 to (Length(data) - 1) do begin
    if i < 8 then begin
      Write(Format(' %d', [data[i]]));
    end;
  end;

  WriteLn('');
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

  { Write standard data frame with identifier 1742 and 3 bytes of data }
  can.WriteFrame(BRICKLET_CAN_V2_FRAME_TYPE_STANDARD_DATA, 1742, [42, 23, 17]);

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
