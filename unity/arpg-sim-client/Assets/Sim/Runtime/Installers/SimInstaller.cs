using Sim.Visuals;
using UnityEngine;
using Zenject;

namespace Sim.Installers
{
    public sealed class SimInstaller : MonoInstaller
    {
        [SerializeField] private SimRunner _simRunner;
        [SerializeField] private Transform _entityRoot;
        [SerializeField] private EntityVisualSettings _entityVisualSettings;
        [SerializeField] private CoreSettings _coreSettings;

        public override void InstallBindings()
        {
            Container.Bind<SimRunner>().FromInstance(_simRunner).AsSingle();
            Container.Bind<EntityVisualSettings>().FromInstance(_entityVisualSettings).AsSingle();
            Container.Bind<CoreSettings>().FromInstance(_coreSettings).AsSingle();
            Container.Bind<Transform>().WithId("EntityRoot").FromInstance(_entityRoot).AsSingle();

            Container.BindInterfacesAndSelfTo<EntityRegistry>().AsSingle();
        }
    }
}