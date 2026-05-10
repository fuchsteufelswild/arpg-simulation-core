using System.Runtime.InteropServices;
using NUnit.Framework;
using Sim.Native;

namespace Sim.Tests
{
    public class SimNativeTests
    {
        [Test]
        public void StructSizes_MatchCAbi()
        {
            Assert.AreEqual(48, Marshal.SizeOf<EntitySnapshotNative>(), "EntitySnapshot");
            Assert.AreEqual(32, Marshal.SizeOf<ProjectileSnapshotNative>(), "ProjectileSnapshot");
            Assert.AreEqual(32, Marshal.SizeOf<DamageEventSnapshotNative>(), "DamageEventSnapshot");
            Assert.AreEqual(48, Marshal.SizeOf<SnapshotNative>(), "Snapshot");
            Assert.AreEqual(28, Marshal.SizeOf<CastAbilityCmdNative>(), "CastAbilityCmd");
            Assert.AreEqual(16, Marshal.SizeOf<MoveIntentCmdNative>(), "MoveIntentCmd");
            Assert.AreEqual(36, Marshal.SizeOf<InputCmdNative>(), "InputCmd");
            Assert.AreEqual(16, Marshal.SizeOf<InputCmdBufferNative>(), "InputCmdBuffer");
            Assert.AreEqual(8,  Marshal.SizeOf<CooldownEntryNative>(), "CooldownEntry");
            Assert.AreEqual(24, Marshal.SizeOf<CooldownSnapshotNative>(), "CooldownSnapshot");
        }

        [Test]
        public void GetVersion_ReturnsNonEmptyString()
        {
            var version = SimNative.GetVersion();
            Assert.IsNotNull(version);
            Assert.IsNotEmpty(version);
            UnityEngine.Debug.Log($"sim_version: {version}");
        }

        [Test]
        public void CreateAdvanceSnapshotDestroy_CompletesCleanly()
        {
            using var handle = SimNative.Create(seed: 12345);
            Assert.IsFalse(handle.IsInvalid);

            SimNative.sim_advance(handle.DangerousGetPtr(), target_tick: 30);

            SimNative.sim_get_snapshot(handle.DangerousGetPtr(), out var snap);
            Assert.AreEqual(30UL, snap.Tick, "Tick should advance to target");
        }

        [Test]
        public void GetCooldowns_OnFreshSim_ReturnsZeroCount()
        {
            using var handle = SimNative.Create(seed: 1);

            SimNative.sim_get_cooldowns(
                handle.DangerousGetPtr(),
                entity_index: 0,
                entity_generation: 0,
                out var cd);

            Assert.AreEqual(0u, cd.Count);
        }
    }
}