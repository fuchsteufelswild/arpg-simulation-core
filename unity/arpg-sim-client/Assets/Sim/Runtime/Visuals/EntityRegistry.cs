using System;
using System.Collections.Generic;
using Sim.Model;
using UniRx;
using UnityEngine;
using Zenject;

namespace Sim.Visuals
{
    public sealed class EntityRegistry : IInitializable, IDisposable
    {
        private readonly SimRunner _simRunner;
        private readonly EntityVisualSettings _settings;
        private readonly Transform _root;

        private readonly Dictionary<EntityHandle, EntityVisual> _visuals = new();
        private readonly HashSet<EntityHandle> _seenThisTick = new();
        private readonly CompositeDisposable _disposables = new();

        public IReadOnlyDictionary<EntityHandle, EntityVisual> Visuals => _visuals;

        public EntityRegistry(
            SimRunner simRunner,
            EntityVisualSettings settings,
            [Inject(Id = "EntityRoot")] Transform root)
        {
            _simRunner = simRunner;
            _settings = settings;
            _root = root;
        }

        public void Initialize()
        {
            _simRunner.SnapshotStream
                .Subscribe(OnSnapshot)
                .AddTo(_disposables);
        }

        public void Dispose()
        {
            _disposables.Dispose();
        }

        private void OnSnapshot(SimSnapshot snapshot)
        {
            _seenThisTick.Clear();

            foreach (var state in snapshot.Entities)
            {
                _seenThisTick.Add(state.Handle);

                if (_visuals.TryGetValue(state.Handle, out var visual))
                {
                    visual.OnSnapshot(state, snapshot.RealtimeReceived);
                }
                else
                {
                    visual = Spawn(state, snapshot.RealtimeReceived);
                    if (visual != null)
                        _visuals.Add(state.Handle, visual);
                }
            }

            DespawnMissing();
        }

        private EntityVisual Spawn(in EntityState state, float realtime)
        {
            var prefab = _settings.GetPrefab((EntityKind)state.EntityKind);
            if (prefab == null)
            {
                Utils.Logger.Warning($"[EntityRegistry] No prefab for kind={state.EntityKind}");
                return null;
            }

            var visual = UnityEngine.Object.Instantiate(prefab, _root);
            visual.name = $"{prefab.name}_{state.Handle.Index}_{state.Handle.Generation}";
            visual.Initialize(state.Handle, state, realtime);
            return visual;
        }

        private void DespawnMissing()
        {
            List<EntityHandle> toRemove = null;
            foreach (var (handle, visual) in _visuals)
            {
                if (_seenThisTick.Contains(handle)) continue;
                toRemove ??= new List<EntityHandle>();
                toRemove.Add(handle);
                UnityEngine.Object.Destroy(visual.gameObject);
            }

            if (toRemove == null) return;
            foreach (var handle in toRemove)
                _visuals.Remove(handle);
        }
    }
}