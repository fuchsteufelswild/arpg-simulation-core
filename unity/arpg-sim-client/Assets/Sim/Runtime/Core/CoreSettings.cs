using UnityEngine;

namespace Sim
{
    [CreateAssetMenu(fileName = "CoreSettings", menuName = "Sim/Core Settings")]
    public sealed class CoreSettings : ScriptableObject
    {
        [SerializeField] private TextAsset _abilitiesToml;
        [SerializeField] private TextAsset _entityArchetypesToml;

        [SerializeField] private uint _seed = 42;
        [SerializeField] private bool _logTicks;

        public TextAsset AbilitiesToml => _abilitiesToml;
        public TextAsset EntityArchetypesToml => _entityArchetypesToml;

        public uint Seed => _seed;
        public bool LogTicks => _logTicks;
    }
}