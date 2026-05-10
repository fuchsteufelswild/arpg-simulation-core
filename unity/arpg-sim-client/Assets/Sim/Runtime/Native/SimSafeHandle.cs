using System;
using System.Runtime.InteropServices;

namespace Sim.Native
{
    /// <summary>
    /// Owns the lifetime of a native SimHandle. Guarantees sim_destroy is called
    /// exactly once, even across domain reloads or unhandled exceptions.
    /// </summary>
    public sealed class SimSafeHandle : SafeHandle
    {
        public SimSafeHandle() : base(IntPtr.Zero, ownsHandle: true) { }

        public override bool IsInvalid => handle == IntPtr.Zero;

        protected override bool ReleaseHandle()
        {
            if (handle != IntPtr.Zero)
            {
                SimNative.sim_destroy(handle);
                SetHandle(IntPtr.Zero);
            }
            return true;
        }

        public IntPtr DangerousGetPtr() => handle;

        internal void SetRawHandle(IntPtr ptr) => SetHandle(ptr);
    }
}