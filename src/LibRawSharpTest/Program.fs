open System
open Aardvark.Base
open System.Runtime.InteropServices
open Microsoft.FSharp.NativeInterop
open LibRaw

[<EntryPoint;STAThread>]
let main argv = 
    Aardvark.Init()

    use img = RawImage.Load @"/Users/hs/IMG_7721.CR2"

    Log.start "image"
    Log.line "make:      %s" img.Make
    Log.line "model:     %s" img.Model
    Log.line "lmake:     %s" img.LensMake
    Log.line "lmodel:    %s" img.LensModel
    Log.line "focal:     %A" img.FocalLength
    Log.line "aperture:  %A" img.Aperture
    Log.line "width:     %A" img.Width
    Log.line "height:    %A" img.Height
    Log.line "data:      %X" img.Data
    Log.stop()
    
    let arr : byte[] = Array.zeroCreate (img.Width * img.Height * 3)
    Marshal.Copy(img.Data, arr, 0, arr.Length)

    let vol =
        Volume<byte>(
            arr,
            0L,
            V3l(img.Width, img.Height, 3),
            V3l(3, 3*img.Width, 1)
        )

    let img = PixImage<byte>(Col.Format.RGB, vol)
    img.SaveAsImage @"C:\Users\Schorsch\Desktop\H1_7\bla.jpg"



    0
