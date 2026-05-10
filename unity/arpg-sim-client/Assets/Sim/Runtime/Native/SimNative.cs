using System;
using System.Runtime.InteropServices;

namespace Sim.Native
{
    public static class SimNative
    {
        private const string DllName = "sim_api";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr sim_version();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr sim_create(uint seed);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void sim_destroy(IntPtr sim);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void sim_advance(IntPtr sim, ulong target_tick);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void sim_submit_commands(IntPtr sim, ref InputCmdBufferNative commands);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void sim_get_snapshot(IntPtr sim, out SnapshotNative out_snapshot);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void sim_get_cooldowns(
            IntPtr sim,
            uint entity_index,
            uint entity_generation,
            out CooldownSnapshotNative out_snapshot);

        public static SimSafeHandle Create(uint seed)
        {
            var ptr = sim_create(seed);
            if (ptr == IntPtr.Zero)
                throw new InvalidOperationException("sim_create returned null");
            var handle = new SimSafeHandle();
            handle.SetRawHandle(ptr);
            return handle;
        }

        public static string GetVersion()
        {
            var ptr = sim_version();
            return ptr == IntPtr.Zero ? null : Marshal.PtrToStringAnsi(ptr);
        }
    }
}