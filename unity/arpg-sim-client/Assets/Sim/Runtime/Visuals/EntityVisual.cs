using Sim.Model;
using UnityEngine;

namespace Sim.Visuals
{
    public sealed class EntityVisual : MonoBehaviour
    {
        private EntityHandle _handle;
        private EntityState _from;
        private EntityState _to;
        private float _fromRealtime;
        private float _toRealtime;
        private bool _hasFrom;

        public EntityHandle Handle => _handle;
        public EntityState Latest => _to;

        public void Initialize(EntityHandle handle, in EntityState initial, float realtime)
        {
            _handle = handle;
            _to = initial;
            _toRealtime = realtime;
            _hasFrom = false;

            transform.position = new Vector3(initial.PosX, 0f, initial.PosY);
            transform.rotation = Quaternion.Euler(0f, -initial.FacingRadians * Mathf.Rad2Deg, 0f);
        }

        public void OnSnapshot(in EntityState state, float realtime)
        {
            _from = _to;
            _fromRealtime = _toRealtime;
            _to = state;
            _toRealtime = realtime;
            _hasFrom = true;
        }

        private void Update()
        {
            if (!_hasFrom)
            {
                transform.position = new Vector3(_to.PosX, 0f, _to.PosY);
                return;
            }

            // Display lags one tick interval behind sim authority.
            // At the moment _to arrives (Time.time == _toRealtime), we display _from.
            // Over the next TickInterval, we interpolate toward _to.
            var elapsedSinceTo = Time.time - _toRealtime;
            var alpha = Mathf.Clamp01((float)(elapsedSinceTo / SimRunner.TickInterval));

            var x = Mathf.Lerp(_from.PosX, _to.PosX, alpha);
            var y = Mathf.Lerp(_from.PosY, _to.PosY, alpha);
            transform.position = new Vector3(x, 0f, y);

            var facing = Mathf.LerpAngle(
                -_from.FacingRadians * Mathf.Rad2Deg,
                -_to.FacingRadians * Mathf.Rad2Deg,
                alpha);
            transform.rotation = Quaternion.Euler(0f, facing, 0f);
        }
    }
}