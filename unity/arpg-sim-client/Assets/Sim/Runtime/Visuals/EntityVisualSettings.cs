using UnityEngine;

namespace Sim.Visuals
{
    [CreateAssetMenu(fileName = "EntityVisualSettings", menuName = "Sim/Entity Visual Settings")]
    public sealed class EntityVisualSettings : ScriptableObject
    {
        [SerializeField] private EntityVisual _playerPrefab;
        [SerializeField] private EntityVisual _enemyPrefab;

        public EntityVisual PlayerPrefab => _playerPrefab;
        public EntityVisual EnemyPrefab => _enemyPrefab;

        public EntityVisual GetPrefab(EntityKind kind) => kind switch
        {
            EntityKind.Player => _playerPrefab,
            EntityKind.Enemy => _enemyPrefab,
            _ => null,
        };
    }
}