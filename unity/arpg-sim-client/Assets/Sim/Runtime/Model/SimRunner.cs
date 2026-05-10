using System;
using Sim.Model;
using Sim.Native;
using UniRx;
using UnityEngine;
using Zenject;

namespace Sim
{
    public sealed class SimRunner : MonoBehaviour
    {
        [SerializeField] private uint _seed = 1;
        [SerializeField] private bool _logTicks;

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
            _handle = SimNative.Create(_seed);
            Utils.Logger.Info($"[SimRunner] Created sim, version={SimNative.GetVersion()}, seed={_seed}");
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

            if (_logTicks)
                Utils.Logger.Info($"[SimRunner] tick={Latest.Tick} entities={Latest.Entities.Count}");
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