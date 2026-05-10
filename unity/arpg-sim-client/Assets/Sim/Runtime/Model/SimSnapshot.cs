using System;
using System.Collections.Generic;

namespace Sim.Model
{
    public readonly struct EntityHandle : IEquatable<EntityHandle>
    {
        public readonly uint Index;
        public readonly uint Generation;

        public EntityHandle(uint index, uint generation)
        {
            Index = index;
            Generation = generation;
        }

        public bool Equals(EntityHandle other) => Index == other.Index && Generation == other.Generation;
        public override bool Equals(object obj) => obj is EntityHandle h && Equals(h);
        public override int GetHashCode() => HashCode.Combine(Index, Generation);
        public static bool operator ==(EntityHandle a, EntityHandle b) => a.Equals(b);
        public static bool operator !=(EntityHandle a, EntityHandle b) => !a.Equals(b);
        public override string ToString() => $"Entity({Index}:{Generation})";
    }

    public readonly struct EntityState
    {
        public readonly EntityHandle Handle;
        public readonly float PosX;
        public readonly float PosY;
        public readonly float FacingRadians;
        public readonly float HealthCurrent;
        public readonly float HealthMax;
        public readonly ulong StatusTags;
        public readonly ushort EntityKind;
        public readonly ushort CastingAbilityId;
        public readonly float CastingProgress;

        public EntityState(
            EntityHandle handle,
            float posX, float posY, float facingRadians,
            float healthCurrent, float healthMax,
            ulong statusTags,
            ushort entityKind,
            ushort castingAbilityId, float castingProgress)
        {
            Handle = handle;
            PosX = posX;
            PosY = posY;
            FacingRadians = facingRadians;
            HealthCurrent = healthCurrent;
            HealthMax = healthMax;
            StatusTags = statusTags;
            EntityKind = entityKind;
            CastingAbilityId = castingAbilityId;
            CastingProgress = castingProgress;
        }
    }

    public readonly struct ProjectileState
    {
        public readonly EntityHandle Caster;
        public readonly float CurrentX;
        public readonly float CurrentY;
        public readonly float TargetX;
        public readonly float TargetY;
        public readonly ushort AbilityId;

        public ProjectileState(
            EntityHandle caster,
            float currentX, float currentY,
            float targetX, float targetY,
            ushort abilityId)
        {
            Caster = caster;
            CurrentX = currentX;
            CurrentY = currentY;
            TargetX = targetX;
            TargetY = targetY;
            AbilityId = abilityId;
        }
    }

    public readonly struct DamageEvent
    {
        public readonly EntityHandle Target;
        public readonly EntityHandle Attacker;
        public readonly float Amount;
        public readonly ulong AbilityTags;
        public readonly bool WasCrit;

        public DamageEvent(EntityHandle target, EntityHandle attacker, float amount, ulong abilityTags, bool wasCrit)
        {
            Target = target;
            Attacker = attacker;
            Amount = amount;
            AbilityTags = abilityTags;
            WasCrit = wasCrit;
        }
    }

    public sealed class SimSnapshot
    {
        public ulong Tick { get; }
        public float RealtimeReceived { get; }
        public IReadOnlyList<EntityState> Entities { get; }
        public IReadOnlyList<ProjectileState> Projectiles { get; }
        public IReadOnlyList<DamageEvent> DamageEvents { get; }

        public SimSnapshot(
            ulong tick,
            float realtimeReceived,
            IReadOnlyList<EntityState> entities,
            IReadOnlyList<ProjectileState> projectiles,
            IReadOnlyList<DamageEvent> damageEvents)
        {
            Tick = tick;
            RealtimeReceived = realtimeReceived;
            Entities = entities;
            Projectiles = projectiles;
            DamageEvents = damageEvents;
        }
    }
}