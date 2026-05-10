using System;
using UniRx;
using Zenject;

namespace Sim.Utils
{
    public static class SignalBusExtensions
    {
        public static IObservable<TSignal> GetStream<TSignal>(this SignalBus signalBus)
        {
            return Observable.FromEvent<Action<TSignal>, TSignal>(
                conversion => conversion.Invoke,
                signalBus.Subscribe<TSignal>,
                signalBus.Unsubscribe<TSignal>);
        }
    }
}