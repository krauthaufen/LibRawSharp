namespace LibRaw

open System
open System.Runtime.InteropServices
open Microsoft.FSharp.NativeInterop

#nowarn "9"

module internal LibRawNative =

    type RawImageHandle =
        struct
            val mutable public Make : nativeptr<byte>
            val mutable public Model : nativeptr<byte>
            val mutable public LensMake : nativeptr<byte>
            val mutable public LensModel : nativeptr<byte>
            val mutable public FocalLength : float32
            val mutable public Aperture : float32
            val mutable public Width : int
            val mutable public Height : int
            val mutable public Data : nativeint
        end

    [<DllImport("LibRawNative.dll")>]
    extern RawImageHandle internal lrLoad(string file)
    
    [<DllImport("LibRawNative.dll")>]
    extern void internal lrFree(RawImageHandle image)

type RawImage internal(handle : LibRawNative.RawImageHandle) =
    let mutable handle = handle

    member x.IsDisposed = handle.Data = 0n

    member x.Make = Marshal.PtrToStringAnsi(NativePtr.toNativeInt handle.Make)
    member x.Model = Marshal.PtrToStringAnsi(NativePtr.toNativeInt handle.Model)
    member x.LensMake = Marshal.PtrToStringAnsi(NativePtr.toNativeInt handle.LensMake)
    member x.LensModel = Marshal.PtrToStringAnsi(NativePtr.toNativeInt handle.LensModel)
    member x.FocalLength = float handle.FocalLength
    member x.Aperture = float handle.Aperture
    member x.Width = handle.Width
    member x.Height = handle.Height
    member x.Data = handle.Data

    static member Load(file : string) =
        new RawImage(LibRawNative.lrLoad file)

    member private x.Dispose(disposing : bool) =
        if disposing then GC.SuppressFinalize x
        if not x.IsDisposed then
            LibRawNative.lrFree handle
            handle.Data <- 0n

    member x.Dispose() = x.Dispose(true)
    override x.Finalize() = x.Dispose(false)

    interface IDisposable with
        member x.Dispose() = x.Dispose()
            
