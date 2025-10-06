import { Link } from 'react-router-dom';
import ThemeToggle from './theme/ThemeToggle';

const milestones = [
  {
    title: 'Interactive scene editor',
    detail: 'Drag and drop geometry, assign materials, and define boundary conditions in 3D.'
  },
  {
    title: 'Dynamic listener paths',
    detail: 'Trace binaural responses as listeners move through your space.'
  },
  {
    title: 'Frequency band analysis',
    detail: 'Inspect custom octave bands, clarity metrics, and energy decay curves.'
  },
  {
    title: 'Collaboration toolkit',
    detail: 'Comment, compare revisions, and export client-ready presentations.'
  }
];

const DemoPage = () => {
  return (
    <div className="relative min-h-screen overflow-hidden bg-slate-50 text-slate-900 transition-colors dark:bg-slate-950 dark:text-slate-100">
      <div aria-hidden className="pointer-events-none absolute inset-0 overflow-hidden">
        <div className="absolute left-[-4rem] top-28 h-64 w-64 rounded-full bg-blue-400/20 blur-3xl dark:bg-blue-500/10" />
        <div className="absolute right-[-6rem] top-[-6rem] h-96 w-96 rounded-full bg-purple-500/15 blur-3xl dark:bg-purple-500/10" />
        <div className="absolute bottom-[-8rem] left-1/2 h-[26rem] w-[26rem] -translate-x-1/2 rounded-full bg-slate-400/10 blur-3xl dark:bg-slate-500/5" />
      </div>

      <div className="relative">
        <header className="sticky top-0 z-20 border-b border-slate-200/60 bg-white/70 backdrop-blur-xl transition dark:border-slate-800/80 dark:bg-slate-950/60">
          <div className="mx-auto flex max-w-6xl items-center justify-between px-4 py-4 lg:px-8">
            <Link to="/" className="text-lg font-semibold tracking-wide">
              <span className="bg-gradient-to-r from-blue-600 via-purple-500 to-sky-500 bg-clip-text text-transparent">BeatFox</span>
            </Link>
            <div className="flex items-center gap-3">
              <ThemeToggle />
              <Link
                to="/"
                className="hidden rounded-full border border-slate-300/80 px-4 py-2 text-sm font-semibold text-slate-700 transition hover:border-slate-400 hover:text-slate-900 dark:border-slate-700 dark:text-slate-200 dark:hover:border-slate-500 md:inline-flex"
              >
                Back to landing
              </Link>
            </div>
          </div>
        </header>

        <main className="mx-auto max-w-6xl px-4 pb-20 pt-16 lg:px-8">
          <div className="grid gap-12 lg:grid-cols-[0.6fr_0.4fr] lg:items-center">
            <div className="space-y-8">
              <p className="inline-flex items-center gap-2 rounded-full border border-slate-200 bg-white/80 px-3 py-1 text-xs font-semibold uppercase tracking-[0.2em] text-slate-600 shadow-sm backdrop-blur dark:border-slate-700 dark:bg-slate-900/70 dark:text-slate-300">
                Demo preview
              </p>
              <div className="space-y-5">
                <h1 className="text-balance text-4xl font-semibold text-slate-900 sm:text-5xl dark:text-white">
                  Immersive acoustic simulation is almost here
                </h1>
                <p className="max-w-xl text-base text-slate-600 dark:text-slate-300">
                  We are building an interactive environment to explore wave propagation, material performance, and listener experience in real time.
                </p>
              </div>
              <div className="space-y-6 rounded-3xl border border-slate-200/70 bg-white/80 p-6 shadow-lg backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
                <h2 className="text-lg font-semibold text-slate-900 dark:text-white">What you will experience</h2>
                <ul className="space-y-4 text-sm text-slate-600 dark:text-slate-300">
                  {milestones.map((item) => (
                    <li key={item.title} className="flex items-start gap-3">
                      <span className="mt-1 h-6 w-6 rounded-full bg-blue-500/10 text-center text-xs font-semibold leading-6 text-blue-600 dark:bg-blue-500/20 dark:text-blue-200">
                        ✓
                      </span>
                      <div>
                        <p className="font-semibold text-slate-800 dark:text-white">{item.title}</p>
                        <p className="mt-1 text-sm">{item.detail}</p>
                      </div>
                    </li>
                  ))}
                </ul>
              </div>
              <div className="flex flex-col gap-3 sm:flex-row">
                <a
                  href="mailto:hello@beatfox.io"
                  className="inline-flex items-center justify-center rounded-full bg-slate-900 px-7 py-3 text-base font-semibold text-white shadow-lg transition hover:-translate-y-0.5 hover:shadow-xl dark:bg-white dark:text-slate-900"
                >
                  Join the beta list
                </a>
                <Link
                  to="/"
                  className="inline-flex items-center justify-center rounded-full border border-slate-300/80 bg-white/70 px-7 py-3 text-base font-semibold text-slate-700 transition hover:border-slate-400 hover:text-slate-900 dark:border-slate-700 dark:bg-slate-900/70 dark:text-slate-200 dark:hover:border-slate-500"
                >
                  Explore the product
                </Link>
              </div>
            </div>

            <div className="relative overflow-hidden rounded-3xl border border-slate-200/70 bg-white/90 shadow-2xl shadow-blue-500/10 backdrop-blur dark:border-slate-800/80 dark:bg-slate-900/80">
              <div className="flex items-center justify-between border-b border-slate-200/70 px-6 py-4 text-sm font-semibold text-slate-600 dark:border-slate-800 dark:text-slate-300">
                <span>Simulation console</span>
                <span className="rounded-full bg-green-500/15 px-3 py-1 text-xs font-semibold text-green-600 dark:bg-green-500/20 dark:text-green-200">
                  In development
                </span>
              </div>
              <div className="space-y-6 px-6 pb-8 pt-6">
                <div className="rounded-2xl border border-slate-200/70 bg-slate-900/90 p-6 text-white dark:border-slate-700 dark:bg-slate-100 dark:text-slate-900">
                  <p className="text-sm uppercase tracking-[0.3em] text-white/70 dark:text-slate-500">Energy map</p>
                  <div className="mt-4 grid grid-cols-3 gap-3">
                    {[32, 58, 74, 48, 90, 63].map((value, index) => (
                      <div
                        key={index}
                        className="aspect-square rounded-xl bg-gradient-to-br from-blue-500/40 to-purple-500/40 text-center text-sm font-semibold leading-[3.5] text-white dark:from-blue-500/20 dark:to-purple-500/20 dark:text-slate-900"
                      >
                        {value}dB
                      </div>
                    ))}
                  </div>
                </div>
                <div className="grid gap-4 sm:grid-cols-2">
                  <div className="rounded-2xl border border-slate-200/70 bg-white/80 p-4 text-sm shadow-sm dark:border-slate-800 dark:bg-slate-900/70">
                    <p className="text-xs font-semibold uppercase tracking-[0.3em] text-slate-500 dark:text-slate-400">Targets</p>
                    <div className="mt-3 space-y-2 text-sm">
                      <div className="flex items-center justify-between">
                        <span>Clarity C80</span>
                        <span className="rounded-full bg-emerald-500/10 px-2 py-1 text-xs font-semibold text-emerald-600 dark:bg-emerald-500/20 dark:text-emerald-200">
                          +2.1 dB
                        </span>
                      </div>
                      <div className="flex items-center justify-between">
                        <span>RT60 @ 1k</span>
                        <span className="rounded-full bg-amber-500/10 px-2 py-1 text-xs font-semibold text-amber-600 dark:bg-amber-500/20 dark:text-amber-200">
                          1.4 s
                        </span>
                      </div>
                      <div className="flex items-center justify-between">
                        <span>STI</span>
                        <span className="rounded-full bg-blue-500/10 px-2 py-1 text-xs font-semibold text-blue-600 dark:bg-blue-500/20 dark:text-blue-200">
                          0.74
                        </span>
                      </div>
                    </div>
                  </div>
                  <div className="rounded-2xl border border-slate-200/70 bg-white/80 p-4 text-sm shadow-sm dark:border-slate-800 dark:bg-slate-900/70">
                    <p className="text-xs font-semibold uppercase tracking-[0.3em] text-slate-500 dark:text-slate-400">Next up</p>
                    <ul className="mt-3 space-y-2">
                      <li className="flex items-center gap-2">
                        <span className="h-2 w-2 rounded-full bg-blue-500" />
                        Field recorder import
                      </li>
                      <li className="flex items-center gap-2">
                        <span className="h-2 w-2 rounded-full bg-purple-500" />
                        Real-time binaural monitor
                      </li>
                      <li className="flex items-center gap-2">
                        <span className="h-2 w-2 rounded-full bg-slate-500" />
                        AI-powered material matcher
                      </li>
                    </ul>
                  </div>
                </div>
              </div>
            </div>
          </div>

          <section className="mt-20 rounded-3xl border border-slate-200/70 bg-white/80 p-10 text-center shadow-xl backdrop-blur dark:border-slate-800 dark:bg-slate-900/70">
            <h2 className="text-3xl font-semibold text-slate-900 dark:text-white">Stay in the loop</h2>
            <p className="mx-auto mt-3 max-w-2xl text-base text-slate-600 dark:text-slate-300">
              Receive release notes, behind-the-scenes insights, and early access invites as we unveil the BeatFox simulation suite.
            </p>
            <div className="mt-8 flex flex-col items-center justify-center gap-3 sm:flex-row">
              <input
                type="email"
                placeholder="Enter your email"
                className="w-full max-w-sm rounded-full border border-slate-300/80 bg-white/70 px-5 py-3 text-sm text-slate-700 shadow-sm transition focus:border-slate-400 focus:outline-none focus:ring-2 focus:ring-blue-400/40 dark:border-slate-700 dark:bg-slate-900/70 dark:text-slate-200"
                disabled
              />
              <button
                type="button"
                className="inline-flex min-w-[180px] items-center justify-center rounded-full bg-slate-900 px-6 py-3 text-sm font-semibold text-white shadow-lg transition dark:bg-white dark:text-slate-900"
                disabled
              >
                Sign up soon
              </button>
            </div>
            <p className="mt-3 text-xs text-slate-500 dark:text-slate-400">Email notifications will open with the beta release.</p>
          </section>
        </main>

        <footer className="border-t border-slate-200/60 bg-white/70 py-8 text-sm text-slate-500 backdrop-blur dark:border-slate-800/80 dark:bg-slate-950/60 dark:text-slate-400">
          <div className="mx-auto flex max-w-6xl flex-col items-center justify-between gap-4 px-4 text-center sm:flex-row sm:text-left lg:px-8">
            <span>© {new Date().getFullYear()} BeatFox Acoustic Intelligence</span>
            <div className="flex flex-wrap justify-center gap-4">
              <Link className="transition hover:text-slate-900 dark:hover:text-white" to="/">
                Landing
              </Link>
              <a className="transition hover:text-slate-900 dark:hover:text-white" href="mailto:hello@beatfox.io">
                Contact
              </a>
            </div>
          </div>
        </footer>
      </div>
    </div>
  );
};

export default DemoPage;
