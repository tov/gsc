'use strict';

import $ from 'jquery'

type Line = HTMLTableCaptionElement | JQuery<HTMLTableCaptionElement>
type JQuerySelect = JQuery<HTMLSelectElement>

const VIEWER_KEY = 'gsc file viewer'

const LINE_NUMBER_KEY = 'gsc line number'

const DIGITS_RE = /[0-9]+/

export function selectCopyId(inputId: string) {
    const elt = document.getElementById(inputId)
    if (elt instanceof HTMLInputElement) {
        elt.select()
        document.execCommand('copy')
    } else {
        console.log(`selectCopyId: could not select ${inputId}`)
    }
}

export class FileViewer {
    private readonly _selector: JQuerySelect
    private readonly _area: JQuery
    private readonly _lines: [Line]
    private readonly _files: [JQuery]

    constructor(element: HTMLElement) {
        const viewer   = $(element)
        const selector = viewer.find('.file-viewer-selector') as JQuerySelect
        const area     = viewer.find('.file-viewer-area')

        this._selector = selector
        this._area     = area
        this._lines    = area.find('td.code-line')
                             .get() as [Line]
        this._files    = area.find('div.single-file-viewer')
                             .get().map($) as [JQuery]

        selector.on('change', this.showSelectedFile.bind(this))
        area.on('scroll', this.selectShownFile.bind(this))
        this.setupLineLinks()

        viewer.data(VIEWER_KEY, this)
    }

    public static forId(id: string): FileViewer {
        const viewer = document.getElementById(id)!
        return $.data(viewer, VIEWER_KEY) ?? new FileViewer(viewer)
    }

    public showLine(lineNo: number) {
        this.showElement(this.line(lineNo), 0.3)
    }

    public showFile(fileNo: number) {
        this.showElement(this.file(fileNo), 0)
    }

    private file(index: number) {
        return this._files[index]
    }

    private line(index: number) {
        const lines = this._lines
        const value = $(lines[index - 1])
        if (value.length) {
            lines[index - 1] = value
        }
        return value
    }

    private setupLineLinks() {
        const lineCount = this._lines.length
        $('.line-link')
            .each((_, link) => {
                const number = extractNumber(link.innerText)
                $.data(link, LINE_NUMBER_KEY, number)
                if (number > lineCount) {
                    link.className = 'broken-line-link'
                }
            })
            .on('click', this.handleLineLink.bind(this))
    }

    private handleLineLink(evt: /*JQuery.*/Event) {
        this.showLine($.data(evt.target!, LINE_NUMBER_KEY))
    }

    private showSelectedFile() {
        const choice = this._selector.val()
        this.showFile(Number(choice))
    }

    private selectShownFile() {
        this._selector.val(this.currentShownFile() ?? 0)
    }

    private currentShownFile() {
        for (const file of this._files) {
            if (bottom(file) >= 0) {
                return file.parent().index()
            }
        }
    }

    private showElement(target: JQuery, margin: number) {
        if (!target.length) {
            this.error()
            return
        }

        const area  = this._area
        const goal  = target.offset()!.top - margin * area.height()!
        const delta = goal - area.offset()!.top
        area.scrollTop(area.scrollTop()! + delta)

        target.effect('highlight', {color: 'purple'}, 500,)
    }

    private error() {
        this._area.effect('shake', {distance: 10, times: 2}, 400)
    }
}

function extractNumber(str: string): number {
    const execResult = DIGITS_RE.exec(str)
    return execResult ? parseInt(execResult[0]) : 0
}

function bottom(elt: JQuery): number {
    return elt.position().top + (elt.height() ?? 0)
}

