using System.Runtime.InteropServices;

namespace Sim.Native
{
    [StructLayout(LayoutKind.Sequential)]
    public struct EntitySnapshotNative
    {
        public uint HandleIndex;
        public uint HandleGeneration;
        public float PosX;
        public float PosY;
        public float FacingRadians;
        public float HealthCurrent;
        public float HealthMax;
        public uint Pad0;
        public ulong StatusTags;
        public ushort EntityKind;
        public ushort CastingAbilityId;
        public float CastingProgress;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ProjectileSnapshotNative
    {
        public uint CasterIndex;
        public uint CasterGeneration;
        public float CurrentX;
        public float CurrentY;
        public float TargetX;
        public float TargetY;
        public ushort AbilityId;
        public ushort Pad0;
        public uint Pad1;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct DamageEventSnapshotNative
    {
        public ulong AbilityTags;
        public uint TargetIndex;
        public uint TargetGeneration;
        public uint AttackerIndex;
        public uint AttackerGeneration;
        public float Amount;
        public ushort Pad1;
        public byte WasCrit;
        public byte Pad0;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SnapshotNative
    {
        public ulong Tick;
        public System.IntPtr Entities;
        public System.IntPtr Projectiles;
        public System.IntPtr DamageEvents;
        public uint EntityCount;
        public uint ProjectileCount;
        public uint DamageEventCount;
        public uint Pad0;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CastAbilityCmdNative
    {
        public uint CasterIndex;
        public uint CasterGeneration;
        public ushort AbilityId;
        public ushort Pad0;
        public uint TargetIndex;
        public uint TargetGeneration;
        public float TargetX;
        public float TargetY;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct MoveIntentCmdNative
    {
        public uint EntityIndex;
        public uint EntityGeneration;
        public float DirectionX;
        public float DirectionY;
    }

    // The C union: largest payload determines size.
    // CastAbilityCmd is 28 bytes, MoveIntentCmd is 16, so union is 28.
    // We use Explicit layout with both variants at offset 8 (after type + pad0).
    [StructLayout(LayoutKind.Explicit, Size = 36)]
    public struct InputCmdNative
    {
        [FieldOffset(0)] public uint Type;
        [FieldOffset(4)] public uint Pad0;
        [FieldOffset(8)] public CastAbilityCmdNative Cast;
        [FieldOffset(8)] public MoveIntentCmdNative Move;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct InputCmdBufferNative
    {
        public uint Count;
        public uint Pad0;
        public System.IntPtr Commands;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CooldownEntryNative
    {
        public ushort AbilityId;
        public ushort Pad0;
        public uint RemainingTicks;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CooldownSnapshotNative
    {
        public uint EntityIndex;
        public uint EntityGeneration;
        public uint Count;
        public uint Pad0;
        public System.IntPtr Entries;
    }
}