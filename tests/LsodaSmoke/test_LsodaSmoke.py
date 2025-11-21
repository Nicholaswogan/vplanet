import astropy.units as u
from benchmark import Benchmark, benchmark


@benchmark(
    {
        "log.final.system.Time": {"value": 3.15576e11, "unit": u.sec},
        "log.final.system.DeltaTime": {"value": 3.15576e10, "unit": u.sec},
        "log.final.gl581.RotPer": {"value": 94.19999999999983, "unit": u.day},
        "log.final.d.RotPer": {"value": 1.00010714185922, "unit": u.day},
        "log.final.d.Eccentricity": {"value": 0.3800000001511541},
    }
)
class Test_LsodaSmoke(Benchmark):
    pass
