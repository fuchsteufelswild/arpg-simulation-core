using UnityEngine;
using Zenject;

namespace Sim.Installers
{
    public sealed class SimInstaller : MonoInstaller
    {
        [SerializeField] private SimRunner _simRunner;

        public override void InstallBindings()
        {
            Container.Bind<SimRunner>().FromInstance(_simRunner).AsSingle();
        }
    }
}