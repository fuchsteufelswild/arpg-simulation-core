using System;
using System.Text;
using Sim.Model;
using Sim.Native;
using UniRx;
using UnityEngine;
using Zenject;

namespace Sim
{
    public sealed class SimRunner : MonoBehaviour
    {
        // We don't rely on Zenject's Injection, as we need Core Settings to be available in Awake
        [SerializeField] private CoreSettings _coreSettings;

        public const double TickInterval = 1.0 / 30.0;

        private SimSafeHandle _handle;
        private double _accumulator;

        private readonly Subject<SimSnapshot> _snapshotStream = new();

        public IObservable<SimSnapshot> SnapshotStream => _snapshotStream;

        public SimSnapshot Latest { get; private set; }
        public ulong TargetTick { get; private set; }

        [Inject]
        public void Construct() { }

        private void Awake()
        {
            _handle = SimNative.Create(_coreSettings.Seed);
            LoadContent(_coreSettings.AbilitiesToml.text, _coreSettings.EntityArchetypesToml.text);
            SpawnArchetype("Player", 0f, 0f);
            Utils.Logger.Info($"[SimRunner] Created sim, version={SimNative.GetVersion()}, seed={_coreSettings.Seed}");
        }

        private void LoadContent(string abilitiesToml, string entityArchetypesToml)
        {
            var abilitiesBytes = Encoding.UTF8.GetBytes(abilitiesToml);
            var archetypesBytes = Encoding.UTF8.GetBytes(entityArchetypesToml);

            var result = SimNative.sim_load_content(_handle.DangerousGetPtr(),
                abilitiesBytes, (UIntPtr)abilitiesBytes.Length,
                archetypesBytes, (UIntPtr)archetypesBytes.Length);

            if (result != 0)
                throw new Exception($"sim_load_content failed");
        }

        private void Update()
        {
            if (_handle == null || _handle.IsInvalid)
                return;

            _accumulator += Time.deltaTime;
            while (_accumulator >= TickInterval)
            {
                _accumulator -= TickInterval;
                TargetTick++;
            }

            SimNative.sim_advance(_handle.DangerousGetPtr(), TargetTick);
            SimNative.sim_get_snapshot(_handle.DangerousGetPtr(), out var native);

            Latest = SnapshotCopier.Copy(native, Time.time);
            _snapshotStream.OnNext(Latest);

            if (_coreSettings.LogTicks)
                Utils.Logger.Info($"[SimRunner] tick={Latest.Tick} entities={Latest.Entities.Count}");
        }

        private ulong SpawnArchetype(string archetypeName, float posX, float posY)
        {
            var handle = SimNative.sim_spawn_archetype(_handle.DangerousGetPtr(), archetypeName, posX, posY);

            return handle == 0
                ? throw new Exception($"Failed to spawn archetype: {archetypeName}")
                : handle;
        }

        private void OnDestroy()
        {
            _snapshotStream.OnCompleted();
            _snapshotStream.Dispose();
            _handle?.Dispose();
            _handle = null;
        }
    }
}