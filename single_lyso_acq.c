
{
c=new TCanvas();
TFile *_file1 = TFile::Open("sa_bkg_lyso.root");
TFile *_file0 = TFile::Open("sa_na_lyso.root");
Tree_na = dynamic_cast<TTree*>(_file0->Get("datatree"));
Tree_bkg = dynamic_cast<TTree*>(_file1->Get("datatree"));
Tree_na->Draw("CH22-502.5+CH23-490+CH21-531.1+CH18-481.4+CH26-361.4>>na_ly(250,0,6400)");
Tree_bkg->Draw("CH22-502.5+CH23-490+CH21-531.1+CH18-481.4+CH26-361.4>>bkg_ly(250,0,6400)");
bkg_ly->SetLineColor(kRed);
na_ly->SetLineColor(kBlue);
bkg_ly->SetTitle("Background");
na_ly->SetTitle("Sodium");
bkg_ly->Draw("");
na_ly->Draw("same");
c->BuildLegend();
c2=new TCanvas ();
Na=dynamic_cast<TH1F*>(na_ly->Clone("Na"));
Na->Add(bkg_ly, -1);
Na->Draw("");
}
