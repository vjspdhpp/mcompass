'use client';
import { Tab, Tabs } from "@heroui/tabs";
import { Card } from "@heroui/card";
import WiFiPanel from "./wifi";
import SpawnPanel from "./spwan";
import InfoPanel from "./info";
import ColorsPanel from "./colors";
import { useState } from "react";

export default function Preference() {

    const [color, setColor] = useState("default");

    return <div className="flex flex-col max-w-xl ">
        <Card className="p-8">
            <Tabs
                aria-label="Options"
                color={color as any}
                onSelectionChange={(key: string) => {
                    if (key === "colors") setColor("default"); // default
                    if (key === "wifi") setColor("success"); // green
                    if (key === "spawn") setColor("primary"); // blue
                    if (key === "info") setColor("danger"); // red
                }}
            >
                <Tab key="colors" title="Colors">
                    <ColorsPanel />
                </Tab>
                <Tab key="wifi" title="WiFi">
                    <WiFiPanel />
                </Tab>
                <Tab key="spawn" title="Spawn">
                    <SpawnPanel />
                </Tab>
                <Tab key="info" title="Info">
                    <InfoPanel />
                </Tab>
            </Tabs>
        </Card>
    </div>;
}