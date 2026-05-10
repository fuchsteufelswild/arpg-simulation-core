using System;
using Sim.Native;

namespace Sim.Model
{
    public static class SnapshotCopier
    {
        public static SimSnapshot Copy(in SnapshotNative native, float realtimeReceived)
        {
            var entities = CopyEntities(native.Entities, native.EntityCount);
            var projectiles = CopyProjectiles(native.Projectiles, native.ProjectileCount);
            var damageEvents = CopyDamageEvents(native.DamageEvents, native.DamageEventCount);

            return new SimSnapshot(
                native.Tick,
                realtimeReceived,
                entities,
                projectiles,
                damageEvents);
        }

        private static EntityState[] CopyEntities(IntPtr ptr, uint count)
        {
            if (count == 0 || ptr == IntPtr.Zero)
                return Array.Empty<EntityState>();

            var result = new EntityState[count];
            unsafe
            {
                var src = (EntitySnapshotNative*)ptr;
                for (var i = 0; i < count; i++)
                {
                    ref var s = ref src[i];
                    result[i] = new EntityState(
                        new EntityHandle(s.HandleIndex, s.HandleGeneration),
                        s.PosX, s.PosY, s.FacingRadians,
                        s.HealthCurrent, s.HealthMax,
                        s.StatusTags,
                        s.EntityKind,
                        s.CastingAbilityId, s.CastingProgress);
                }
            }
            return result;
        }

        private static ProjectileState[] CopyProjectiles(IntPtr ptr, uint count)
        {
            if (count == 0 || ptr == IntPtr.Zero)
                return Array.Empty<ProjectileState>();

            var result = new ProjectileState[count];
            unsafe
            {
                var src = (ProjectileSnapshotNative*)ptr;
                for (var i = 0; i < count; i++)
                {
                    ref var s = ref src[i];
                    result[i] = new ProjectileState(
                        new EntityHandle(s.CasterIndex, s.CasterGeneration),
                        s.CurrentX, s.CurrentY,
                        s.TargetX, s.TargetY,
                        s.AbilityId);
                }
            }
            return result;
        }

        private static DamageEvent[] CopyDamageEvents(IntPtr ptr, uint count)
        {
            if (count == 0 || ptr == IntPtr.Zero)
                return Array.Empty<DamageEvent>();

            var result = new DamageEvent[count];
            unsafe
            {
                var src = (DamageEventSnapshotNative*)ptr;
                for (var i = 0; i < count; i++)
                {
                    ref var s = ref src[i];
                    result[i] = new DamageEvent(
                        new EntityHandle(s.TargetIndex, s.TargetGeneration),
                        new EntityHandle(s.AttackerIndex, s.AttackerGeneration),
                        s.Amount,
                        s.AbilityTags,
                        s.WasCrit != 0);
                }
            }
            return result;
        }
    }
}