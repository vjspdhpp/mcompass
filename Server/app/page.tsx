import Preference from "./preference";

export default function Home() {

  return (
    <section className="flex flex-col items-center justify-center gap-4 py-8 md:py-10">
      {/* <div className="inline-block max-w-xl text-center justify-center">
        <span className={title()}>Bring the&nbsp;</span>
        <span className={title({ color: "violet" })}>Compass&nbsp;</span>
        <br />
        <span className={title()}>
          from Minecraft to Life.
        </span>
        <div className={subtitle({ class: "mt-4" })}>
          Set target Location.
        </div>
        </div> */}
        <Preference />

      <div className="flex gap-3">
        {/* <Link
          isExternal
          className={buttonStyles({
            color: "primary",
            radius: "full",
            variant: "shadow",
          })}
          href={siteConfig.links.docs}
        >
          Documentation
        </Link> */}

      </div>

      {/* <div className="mt-8">
        <Snippet hideCopyButton hideSymbol variant="bordered">
          <span>
            Get started by editing <Code color="primary">app/page.tsx</Code>
          </span>
        </Snippet>
      </div> */}
    </section>
  );
}
